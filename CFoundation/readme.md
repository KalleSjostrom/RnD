## Setting up the c-api runtime

### Plugin API
The *Plugin API* contains the header files necessary for the game plugin to communicate with *Stingray*. If you have the environment variable `SR_SOURCE_DIR` (e.g. `D:/work/Stingray`) set up, the build scripts will use the plugin API from the stingray source. If it is not set up, the build scripts will use the one found in `../binaries/plugin_api`.

__Note: `binaries/plugin_api` is updated by running `update_binaries.bat` and are linked to that version of the engine binaries.__

### Plugin Environment
The Plugin Environment contains all of the necessary includes, libs and binaries to compile the generators and the game plugin, it can be found in `../plugin_environment` (relative to the project folder) and is updated by running `update_binaries.bat`.

## Building
There are several batch scripts to use when building (found in the `foundation` folder)
+ `setup.bat` - clean, build and run all generators, as well as the game
+ `run_all_generators.bat` - run all generators and builds the game
+ `build.bat` - runs crucial generators and builds the game
+ `clean.bat` - removes all compiled and generated files. It won't remove the dll from the engine plugin folder.

We use unity builds at the time being, so all .h or .cpp files should be included only once.

#### Foundation
The foundation source is precompiled using precompiled headers `*.pch`). If changing or adding code to foundation you will need to run `foundation/build.bat` or `run_all_generators.bat`

## Using Visual Studio
### Generating Visual Studio project
* Go into `foundation/codegen/generate_visual_studio_project`
* Run `build.bat` if needed and then `generate.exe`
* This will `output game_plugin.vcxproj` and `game_plugin.vcxproj.filters`
* In visual studio, click the solution and `Add/Existing Project...` and browse to the `game_plugin.vcxproj`

### Exceptions
Our plugin is updated within a `__try` block if a debugger is attached. This because we don't want the engine to go down if our game code crash. For this to actually do anything useful, visual studio must be setup to catch these exceptions. Go to

* __DEBUG/Exceptions...__
* Check the __Win32__ Exceptions box.

This will cause visual studio to halt on exceptions such as access violation, giving you a chance to investigate, and when you hit continue the plugin manager will unload the plugin and try to reload it as soon as you recompile it. You will get a couple of halts for First-chance exceptions in `game.cpp` (if console is on) and in `physx_sdk.cpp`. Just uncheck exceptions for these particual errors.

### Inspecting IdString32/IdString64
To see the actual strings instead of the hash when debugging in visual studio, run `Stingray/tools/visual_studio_plugins/install.rb`.

### Visual studio and sublime text
To be able to open a file in sublime text from visual studio, follow this guide:
  http://stackoverflow.com/questions/18450402/moving-from-visual-studio-to-sublime-text

## Sublime tools
### Building Hammerfall
With the lastest *Sublime Tools*, goto *Tools/Build System* and select `Hammerfall`, with this you can just run `ctrl+b` to build the game dll. You can select witch *build-variant* to use with `ctrl+shift+b`. The last picked variant is stored, so all subsequent ctrl+b (and save-reloads) will use it. If the game is running, it will reload the game. F4 steps through any errors. If there are errors with the generation, (either the generator command syntax is wrong or there is a bug in the generator) this output will contain both the line/number of the error that the generator found and the line/number in the generator source where the error was found.

To change build configuration (dev/debug) or platform (win/ps4), you need to edit your sublime project settings (or have multiple and quick switch between them)

* `"build_configuration": "dev",`
* `"build_platform": "win",`

You can also compile game data for the specified platform with a build variant. This is particularly useful for the ps4 since running the visual studio project won't compile game data for ps4.

### Reloading Hammerfall on save
If you have `rebuild_gamedll_on_save = true` in your sublime settings, it will rebuild on save. This version is the same as build but won't generate as much code (only update the data-layout for reloading).

## Ps4
Assuming that you have the Ps4 SDK installed and the neighborhood setup, you should be able to build and run for the ps4. You will need a file server to transfer the file to the ps4. This can be activated by running the stingray editor or just the backend. If running from visual studio, you will need to supply 

* `--host` [file server ip (your local ip if running yourself)]
* `--secret` [a hash found in the ps4 compile output directory in a file called `.stingray-asset-server-secret`]

If running through sublime, these needs to be set in the settings instead. For example:

* `"file_server_ip": "192.168.1.101",` 
* `"file_server_secret": "86d9ac98515bf61274d1714e60ecdd1a",` 

## Generators/meta scripting
We employ a meta scripting system to generate code, flow nodes, network configs and stingray component files.

### Generator-scripts
All generators are based on the same principles and uses the same utils (`utils/common.cpp`).

`common.cpp` broadly consists of:
+ `memory_arena.cpp` - A simple and fast memory allocator that only uses pointer-bumping
+ `parser.cpp` - A tokenization based parser
+ `profiler.c` - Used in debug to profile the generators
+ `string_utils.inl` - Declares the String type used. It is based on a char* and length, not null terminated strings which makes e.g. comparing a bit faster in the general case
+ some common functions for checking file/directory-existence, hash lookup, converting string cases (e.g. `make_underscore_case, to_pascal_case`)

A generator has the following lifecycle
+ Starts by allocating all memory it needs from the system. (if it runs out, it crashes).
+ Reads cached data from previous runs.
+ Iterating over the files it cares about. (it uses `FindFirstFile` from a starting location and avoids ignored folders).
+ Parses the file (if not in cache), token by token and extracts the parts it wants. These functions and files are always prefixed with `parse_`.
+ Sometimes, it needs to coalesce the parsed data. If not needed then this step is obmitted.
+ Writes parsed data to cache. 
+ Outputs generated files. All generated files are named \*.generated.\*. The output functions and files are always prefixed with `output_`.

A simple example would be the `generate_game_strings.cpp` that outputs prehashed game strings.
+ Allocates the storage for game strings and the hashmap for avoiding duplicates.
+ Reads the cached strings from previous runs.
+ Calls iterate on the folders of interest.
+ Calls `parse_for_game_strings.cpp` which goes through the file, token by token and looks for `ID`, `ID32` and `ID64`. If so adds them to the storage.
+ No need to coalesce anything.
+ Writes the strings it found in to the cache as well as where and when it found them.
+ Writes the unique strings and their hashes to `game_strings/game_strings.generated.cpp`.

### Command markers:
__NOTE: Only used in h-files__

The marker `//!` is called a command marker and used to tells the generation scripts some information about what it is parsing. For example `//! component` means that the following namespace represents component information. A command marker can take paramaters, e.g. `//! component max(64)` means that we can only have 64 instances of this component. The command markers are listed below:

##### Component markers
+ `component` - a new component declaration, needs to be above a namespace containing one ore more _component structs_.
  - `max(number)` - max number of instances that the component can have.
  - `after(component, ...)` - _OPTIONAL_ - specifies a list of components that must be update before this. (this component must come after those components.)
  - `depends_on(component, ...)` - _OPTIONAL_ - specifies a list of components that must be present on an entity if this is.
+ `default_value` - the default value of a component struct member. It will use the rest of the line.

##### Event markers
These markers should be placed before a function in the h-file. When doing so, it will get added to the event delegate and a registration funtion is created which needs to be called once per instance.

`void EventDelegate::register_some_object_name(void *object);`

+ `event` - placed before a function prefixed with `on_`.
+ `flow` - placed before a function prefixed with `flow_`. This will generate a flow node in `code/generated/generated.script_flow_nodes` and this function is called when the flow node is triggered.
  - `delayed_return(event some_event_name1, ...)` - _OPTIONAL_ - indicates that the out events from the flow node should not be triggered automatically. This node will store them and trigger them at a later time.
  - `map(some_name1, some_name2, ...)` - _OPTIONAL_ - creates a flow node with a function_map. The given names are the names of input as seen in flow. The first parameter to the c-function is an unsigned corresponding to which of the in-events got triggered (0 being the first).
  - `return(some_flow_type some_name1, ...)` - _OPTIONAL_ creates a flow node with return values.
+ `rpc` - placed before a function prefixed with `rpc_`. Adds the function to the `generated.network_config`.
  - `not_session_bound` - _OPTIONAL_ - rpcs are session bound by default, use this flag to mark the rpc as not session bound

##### Reload markers
+ `reloadable` - the following struct is reloadable, i.e. it's data layout can change during reloads
  - `on_added('construct' or 'zero')` - _OPTIONAL_ - default is 'construct' - if this struct is added to another reloadable struct between reloads, should it just zero out the memory or run the default constructor
+ `count` - specifies how many elements of an array that should be copied on reload
  - `some_name` - this is a name of the variable in the same struct as the array that contains the number of elements to copy
+ `namespace` - tells the reloader that this variable type is declared inside the given namespace
  - `some_name` - the name of the namespace

#### Command blocks:
__NOTE: Only used in h-files__

The marker block `/*! */` is called a command block. They are basically command markers but support multiline.
+ `export` - Exports all the text until end of block and associate that with the member it is declared above. For example, in a Network component struct:

```cpp
/*! export
bits = 20
min = -999 */
Vector3 position;
```
will export
```cpp
_somecomponentname_position = {
    type = "vector3"
    bits = 20
    min = -999
}
```
to `generated.network_config`

## Components
A component is declared in a namespace inside a _component.h file. This namespace will contain one or more of component structs. These are:
* __Network__ - network syncs its members via game object fields.
* __Master__ - only available to the owner of the entity.
* __MasterInput__ - same as _Master_ but will also generate functions for setting these from other components.
* __Slave__ - available to all clients (including owner).
* __SlaveInput__ - same as _Slave_ but will also generate functions for setting these from other components.
* __Static__ - static settings such as max_hitpoints that all instances can share.

## Reloading
When the game is started, a snapshot of the data-layout is output to `typeinfo.generated.cpp` (if compiled with `RELOAD` defined). When the game is then reloaded we allocate a new chunk of memory, go through the old memory, member by member, comparing the old layout with the current one and move everything to it's correct place in the new layout. Then we deallocate the old chunk, and take a new snapshot.

If a struct has a pointer to another game struct, it has to be reloadable otherwise it's pointer won't get updated to point into the new memory space.

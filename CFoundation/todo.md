## TODO
+ Exception handling on ps4 (replace the __try()/__except block with try/catch block on ps4.)
+ engine <-> game dll version test
+ Remove hard-coded paths in codegen/ability/generate_ability.cpp
+ Debug/?/Release builds
+ FIX: Asynchronous loading

## DONE
+ Remove // Automatically generated at 2016-05-26 10:59:56
+ Improve handling of reload vs non-reload builds. Maybe separate into two scripts, build_game.bat, reload_game.bat?
+ Skulle vi kunna lägga till en update_binaries_remote.bat?
+ Flow event parse
+ Animation event parse
+ FIX: _Application.set_plugin_hot_reload_directory("D:/Work/Hammerfall/.dll_staging_for_hotreload");
+ rename .dll_staging_for_hotreload to .hotreload
+ Reloader.py merge with hammerfall reloader
+ What happens with dll reloading if you have multiple engine instances running, all having the moduled loaded
+ Can we make our own Default/exec.py script for running build systems? (target = "Default/exec.py")
+ FIX: Crash in lua when console is running and starting game
+ Currently not possible to perform HASH_LOOKUP on a go_id of a just spawned entity while in on_added(), as the go_id has not been added to the goid_instance_hashmap (talk to Linus for more info)
+ Command line parameter parsing
+ Flow Lua reference is not a function: `WwiseFlowCallbacks.wwise_unit_load_bank` (Global table not found: `WwiseFlowCallbacks`)

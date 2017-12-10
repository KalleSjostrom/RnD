// Information and examples about the features of the C foundation and code generation

/********************************************************************************************************************
	General
********************************************************************************************************************/

	// Animation events are generated as enums and can be used such as:
	_Unit.trigger_event(unit, AnimationEvent_sprint)

	// Flow events are generated as enums and can be used such as:
	_Unit.trigger_flow_event(unit, FlowEvent_on_dropped)

	// Assets are generated as id strings and can be used such as:
	component_manager.spawn_entity(content::characters::player::player /* content/characters/player/player */, sc);

	// Game strings are generated as IdString32/64 and can be used such as:
	IdString32 game_string = ID(game_string)
	IdString32 game_string = ID32(game_string, "game_string")
	IdString64 game_string = ID64(game_string, "game_string")
	// To convert a game string id to const char*:
	const char* game_string    = ID_STR32(game_string)
	const char* game_string_32 = ID_STR32(game_string_32)
	const char* game_string_64 = ID_STR64(game_string_64)

	// Various useful macros are defined in managers/component_manager.h

/********************************************************************************************************************
	Components
********************************************************************************************************************/

	// Supported flags are:
	//   max(number) - the max amount of entity instances with this component
	//   icon(string) - the icon used for the component in the editor (based on http://fontawesome.io/icons/)
	//   after(component1, component2, ...) - this component must come after the specified components in the order they are updated
	//   depends_on(component1, component2, ...) - entities with this component must also have the specified components

	//! component max(8) icon(male) after(input) depends_on(inventory, unit, rotation, motion)
	namespace avatar {
		// The Master struct holds the state of the master part of the component (per-entity)
		struct Master {
			//! default_value 1.0f
			float value;
			...
		};

		// The MasterInput struct holds the state of the master input part of the component (per-entity)
		// The members of MasterInput will generate a method named input_variablename(EntityRef entity, VariableType variablename)
		// The following ...
		struct MasterInput {
			InputMessage input;
		};
		// ... generates ...
		void input_input(EntityRef entity, InputMessage input) { ... }
		// ... which allows the MasterInput.input variable to be set by calling
		INPUT(entity, avatar, input, InputData())

		// The Slave struct holds the state of the slave part of the component (per-entity)
		struct Slave {
			...
		};

		// The Network struct holds the state of the network part of the component (per-entity)
		struct Network {
			...
		};

		// The Static struct holds the settings part of the component (const, per-entity type)
		struct Static {
			...
		};
	}

	class AvatarComponent {
	public:
		// Method declarations and component members go here ..

		bool handle_actions(unsigned index, unsigned allowed_actions, float dt);

		CharacterStateMachine state_machine;

		//! BEGIN_GENERATED
		// Generated code
	}

/********************************************************************************************************************
	Events
********************************************************************************************************************/

	// Define event
	//! event
	void game_object_sync_done(PeerId sending_peer);
	// Trigger event:
	event_delegate->trigger_game_object_sync_done(sending_peer);

	// Entity events ...
	//! entity_event_trigger on_damage(EntityRef entity, int damage);

	// ... are used with these macros
	REGISTER_ENTITY_EVENT(name, entity, object, receiver)
	TRIGGER_ENTITY_EVENT(name, entity, object)
	UNREGISTER_ENTITY_EVENT(name, entity, ...)

/********************************************************************************************************************
	Network
********************************************************************************************************************/

	// Game object fields
	// Supported field types are: bool, int, float, Vector3, Quaternion
	// Supported export params are: bits, min, max, tolerance
	// Must be defined in a component's Network struct

	// The following code ...
	namespace somecomponentname {
		struct Network {
			/*! export
			bits = 20
			min = -999*/
			Vector3 position;
		}
	}
	// ... will export this type to generated.network_config
	_somecomponentname_position = {
	    type = "vector3"
	    bits = 20
	    min = -999
	}

	// To set the value of a game object field:
	SET_GO_FIELD(instance_index, go_field_value, go_field_value);

	// RPCs
	// Supported param types are: bool, int, float, Vector3, Quaternion, string, PeerID and types exported from network/network.conversions
	// Supported flags are: not_session_bound
	// Must be defined in a header file

	// To specify an RPC:
	//! rpc not_session_bound
	void from_server_spawn_position(PeerId sender, int index, Vector3 position, Quaternion rotation);
	// To send an RPC:
	network_router::send::from_server_spawn_position(sender, index, position, rotation);

	// Arrays can be used in RPCs by tagging a struct with 'network_type'
	// The struct must contain members named 'entries' and 'count'
	//! network_type
	struct IntArray {
	    int entries[8];
	    unsigned count;
	};
	//! network_type
	struct TestArray {
	    IntArray entries[8];
	    unsigned count;
	};

/********************************************************************************************************************
	Settings
********************************************************************************************************************/

	// Must be defined in a component's Static struct
	namespace somecomponentname {
		struct Static {
			/*! export
			default = 4
			editor = { label = "Movement speed"; description = "Avatar movement speed"; }*/
			float move_speed;
		}
	}

	// A custom struct can be used for settings by tagging it with 'settings' ...
	// (the structs must be defined in a separate header, not in the component's header)
	//! settings
	struct InputSettings {
		/*! export
		default = 0.3
		editor = { label = "Prone hold time"; description = "The time needed to hold down the Switch Stance button until prone is toggled"; }*/
		float hold_stance_button_time_for_prone;
	};

	// ... which can then be used as:
	namespace somecomponentname {
		struct Static {
			/*! export
			editor = { label = "InputSettings"; description = "Contains information about avatar input handling"; } */
			InputSettings input_info;
		}
	}

	// Enums can be used for settings by tagging it with 'settings'
	//! settings
	enum SomeEnum { ... }


/********************************************************************************************************************
	State machine
********************************************************************************************************************/

	// A state machine can be generated by tagging a struct (representing a state) with 'state(NameOfStateMachine)'
	//! state(MenuScreen)
	struct MenuScreenMain {
		...
	}

	// The state machine interface is specified in a .game_state_machine located in the same directory as the states
	MenuScreen {
		void finish_reload();
		void on_enter(StateMenu *menu, void *params);
		void on_exit();
		void update(GuiPtr gui, float dt);
		void input(Input *input);
	};
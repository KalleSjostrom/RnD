namespace debug {
	#include "debug_menu.cpp"

	static const unsigned TEXT_LAYER = 100;
	static const float FONT_SIZE = 16.0f;
	static const float NOTIFICATION_FONT_SIZE = 48.0f;
	static const float NOTIFICATION_TIME = 3.0f;

	static const Vector4 DEFAULT_TEXT_COLOR = vector4(255, 255, 255, 255);
	static const Vector4 DEFAULT_COLOR = vector4(255, 71, 146, 71);
	static const Id64 FONT_SMALL = ID64(core__performance_hud__debug, "core/performance_hud/debug");

	static const unsigned NUM_DRAWERS = 512;
	static const unsigned NUM_TEXT_OBJECTS = 128;
	static const unsigned NUM_FEED_TEXT_OBJECTS = 128;
	static const unsigned NUM_WORLD_TEXT_OBJECTS = 128;
	static const unsigned NUM_NOTIFICATION_TEXT_OBJECTS = 8;

	struct TextInfo {
		char text[1024];
		Vector4 color;
	};

	struct FeedTextInfo {
		char text[1024];
		Vector4 color;
		float lifetime;
	};

	struct WorldTextInfo {
		char text[1024];
		Vector4 color;
		Vector2 position;
	};

	struct NotificationTextInfo {
		char text[512];
		Vector4 color;
		float width;
		float time;
	};

	class Drawer;

	DEPRECATED_HASH_ENTRY(DrawerEntry, unsigned, LineObjectPtr);
	DEPRECATED_HASH_MAP(DrawerHashMap, DrawerEntry, 512)

	class Debug {
	public:
		Debug() : disabled(true),
					world(0), viewport(0), shading_environment(0), camera_unit(0), camera(0), camera_to_follow(0),
				  	num_drawers(0), num_texts(0), num_feed_texts(0), num_world_texts(0), num_notifications(0), debug_menu() {

			WorldConfig world_config;
			world_config.disable_physics = true;
			world_config.disable_sound = true;
			world = _Application.new_world(&world_config);

			viewport = _Application.create_viewport(world, ID(stingray_debug));
			shading_environment = _World.create_shading_environment(world, ID64(core_midday, "core/stingray_renderer/environments/midday/midday"));

			Matrix4x4 tm = utils::from_quaternion_translation(quaternion_identity(), vector3(0, 0, 0));
			camera_unit = _World.spawn_unit(world, ID64(core_camera, "core/units/camera"), &tm);
			camera = _Unit.camera(camera_unit, 0);

			gui = _World.create_screen_gui(world, 0, 0, 0, 1, 0, 0);
			material = _Gui.material(gui, ID64(core__performance_hud__debug, "core/performance_hud/debug"));

			DEPRECATED_HASH_INIT(drawer_hashmap, DrawerEntry, 0, 0);

			resolution = _Gui.resolution(0, 0);

			debug_menu.set_gui(gui);
		}

		~Debug() {}

		Drawer *drawer(IdString32 id, bool disable_depth_test = true) {
			DEPRECATED_HASH_LOOKUP(entry, drawer_hashmap, id.id());
			if (entry->key != 0)
				return (Drawer*)&entry->value;

			LineObjectPtr line_object = _World.create_line_object(world, disable_depth_test);
			entry->value = line_object;
			entry->key = id.id();

			// We also store the drawer in a linear array for fast iteration
			// Drawer d;
			// d.line_object = line_object;
			drawers[num_drawers++] = line_object;

			return (Drawer*)&entry->value;
		}

		bool enabled(){
			return !disabled;
		}

		void enable(){
			disabled = false;
		}

		void disable(){
			clear();
			disabled = true;
		}

		void clear() {
			for (unsigned i = 0; i < num_drawers; ++i) {
				LineObjectPtr line_object = drawers[i];
				_LineObject.reset(line_object);
				_LineObject.dispatch(world, line_object);
			}
			ZERO_STRUCT(drawer_hashmap);
			num_drawers = 0;
			num_texts = 0;
			num_feed_texts = 0;
			num_notifications = 0;
		}

		#define _INSERT_TEXT(_info, _format) { \
			va_list ap; \
			va_start(ap, (_format)); \
			vsnprintf((_info).text, ARRAY_COUNT((_info).text), (_format), ap); \
			va_end(ap); \
		}

		void world_text_color(Vector3 world_position, Vector4 color, const char *format, ...) {
			if(disabled) return;
			if(num_world_texts >= NUM_WORLD_TEXT_OBJECTS) return;
			Vector3 screen = _Camera.world_to_screen(camera_to_follow, &world_position);
			if(screen.z > 1.0f) return;

			WorldTextInfo &info = world_texts[num_world_texts++];
			_INSERT_TEXT(info, format);

			info.color = color;
			info.position.x = screen.x;
			info.position.y = screen.y;
		}
		void world_text(Vector3 world_position, const char *format, ...) {
			if(disabled) return;
			if(num_world_texts >= NUM_WORLD_TEXT_OBJECTS) return;
			Vector3 screen = _Camera.world_to_screen(camera_to_follow, &world_position);
			if(screen.z > 1.0f) return;

			WorldTextInfo &info = world_texts[num_world_texts++];
			_INSERT_TEXT(info, format);

			info.color = DEFAULT_TEXT_COLOR;
			info.position.x = screen.x;
			info.position.y = screen.y;
		}

		void unit_text(UnitRef unit, const char *format, ...) {
			if(disabled) return;
			if(num_world_texts >= NUM_WORLD_TEXT_OBJECTS) return;

			OOBB oobb = _Unit.box(unit);
			Vector3 world_position = translation(oobb.tm);
			Vector3 screen_position = _Camera.world_to_screen(camera_to_follow, &world_position);
			if(screen_position.z > 1.0f) return;

			WorldTextInfo &info = world_texts[num_world_texts++];
			_INSERT_TEXT(info, format);

			info.position = vector2(screen_position.x, screen_position.y);
			info.color = DEFAULT_TEXT_COLOR;
		}

		void text(Vector4 color, const char *format, ...) {
			if(disabled) return;
			if(num_texts >= NUM_TEXT_OBJECTS) return;

			TextInfo &info = texts[num_texts++];
			_INSERT_TEXT(info, format);
			info.color = color;
		}

		void text(const char *format, ...) {
			if(disabled) return;
			if(num_texts >= NUM_TEXT_OBJECTS) return;

			TextInfo &info = texts[num_texts++];
			_INSERT_TEXT(info, format);
			info.color = DEFAULT_TEXT_COLOR;
		}

		void feed_text(const char *format, ...) {
			if(disabled) return;
			if(num_feed_texts >= NUM_FEED_TEXT_OBJECTS) return;

			FeedTextInfo &info = feed_texts[num_feed_texts++];
			_INSERT_TEXT(info, format);
			info.color = DEFAULT_TEXT_COLOR;
			info.lifetime = 10;
		}
		void feed_text(Vector4 color, const char *format, ...) {
			if(disabled) return;
			if(num_feed_texts >= NUM_FEED_TEXT_OBJECTS) return;

			FeedTextInfo &info = feed_texts[num_feed_texts++];
			_INSERT_TEXT(info, format);
			info.color = color;
			info.lifetime = 10;
		}

		void notify(const char *format, ...) {
			if(disabled) return;
			if(num_notifications >= NUM_NOTIFICATION_TEXT_OBJECTS) return;

			NotificationTextInfo &info = notifications[num_notifications++];
			_INSERT_TEXT(info, format);
			TextExtentsResult results = _Gui.text_extents(gui, info.text, FONT_SMALL, NOTIFICATION_FONT_SIZE, 0);
			info.color = DEFAULT_TEXT_COLOR;
			info.width = results.max.x - results.min.x;
			info.time = NOTIFICATION_TIME;
		}

		void update(float dt) {
			if (!disabled){
				if (camera_to_follow) {
					ConstLocalTransformPtr pose = _Camera.local_pose(camera_to_follow);
					_Camera.set_local_pose(camera, camera_unit, pose);

					float fov = _Camera.vertical_fov(camera_to_follow);
					_Camera.set_vertical_fov(camera, fov);
				}

				resolution = _Gui.resolution(0, 0);

				{ // Screen texts
					Vector2 position = { 10, resolution.y - 5 };
					for (unsigned i = 0; i < num_texts; ++i) {
						position.y -= (FONT_SIZE); // TODO(kalle): Get proper line height
						TextInfo &text_info = texts[i];

						_Gui.text(gui, text_info.text, FONT_SMALL, FONT_SIZE, material, &position, TEXT_LAYER, 0, &text_info.color);
					}
					num_texts = 0;
				}

				{ // World texts
					for (unsigned i = 0; i < num_world_texts; ++i) {
						WorldTextInfo &world_text_info = world_texts[i];

						_Gui.text(gui, world_text_info.text, FONT_SMALL, FONT_SIZE, material, &world_text_info.position, TEXT_LAYER, 0, &world_text_info.color);
					}
					num_world_texts = 0;
				}

				{ // Feed texts
					Vector2 position = { 10, (float)(10 + num_feed_texts * (FONT_SIZE)) };
					unsigned num_removed = 0;
					for (unsigned i = 0; i < num_feed_texts; ++i) {
						position.y -= (FONT_SIZE);
						FeedTextInfo &feed_text_info = feed_texts[i];

						feed_text_info.lifetime -= dt;
						if(feed_text_info.lifetime < 0) {
							num_removed++;
						}

						Vector4 color = feed_text_info.color;
						color.a = math::saturate(feed_text_info.lifetime) * 255.0f;

						_Gui.text(gui, feed_text_info.text, FONT_SMALL, FONT_SIZE, material, &position, TEXT_LAYER, 0, &color);
					}

					if(num_removed > 0) {
						for (unsigned i = num_removed; i < num_feed_texts; ++i) {
							memcpy(&feed_texts[i-num_removed], &feed_texts[i], sizeof(FeedTextInfo));
						}
						num_feed_texts -= num_removed;
					}
				}

				{ // Notifications
					Vector2 position = { 0, resolution.y/2 };
					for (unsigned i = 0; i < num_notifications; ++i) {
						NotificationTextInfo &text_info = notifications[i];

						position.x = (resolution.x - text_info.width)/2.0f;

						text_info.time -= dt;
						if (text_info.time <= 0) {
							notifications[i--] = notifications[--num_notifications];
						} else {
							Vector4 &color = text_info.color;
							color.x = math::saturate(text_info.time) * 255.0f;
							_Gui.text(gui, text_info.text, FONT_SMALL, NOTIFICATION_FONT_SIZE, material, &position, TEXT_LAYER, 0, &color);
							position.y -= NOTIFICATION_FONT_SIZE;
						}
					}
				}

				{ // Dispatch line objects
					for (unsigned i = 0; i < num_drawers; ++i) {
						LineObjectPtr line_object = drawers[i];
						_LineObject.dispatch(world, line_object);
					}
				}
			}

			debug_menu.update(dt);
			_World.update(world, dt);
		}

		void render() {
			_Application.render_world(world, camera, viewport, shading_environment, 0);
		}

		/// member variables
		bool disabled;

		WorldPtr world;
		ViewportPtr viewport;
		ShadingEnvironmentPtr shading_environment;
		UnitRef camera_unit;
		CameraPtr camera;
		CameraPtr camera_to_follow;
		GuiPtr gui;
		MaterialPtr material;
		Vector2 resolution;

		debug_menu::DebugMenu debug_menu;

		DrawerHashMap drawer_hashmap;

		LineObjectPtr drawers[NUM_DRAWERS];
		unsigned num_drawers;

		TextInfo texts[NUM_TEXT_OBJECTS];
		unsigned num_texts;

		FeedTextInfo feed_texts[NUM_FEED_TEXT_OBJECTS];
		unsigned num_feed_texts;

		WorldTextInfo world_texts[NUM_WORLD_TEXT_OBJECTS];
		unsigned num_world_texts;

		NotificationTextInfo notifications[NUM_NOTIFICATION_TEXT_OBJECTS];
		unsigned num_notifications;

		AllocatorApi *allocator_api;
		AllocatorObject *allocator_object;
	};

	static Debug *_debug_object;

	void *init(GetApiFunction get_engine_api) {
		AllocatorApi *allocator_api = (AllocatorApi*) get_engine_api(ALLOCATOR_API_ID);
		AllocatorObject *allocator_object = allocator_api->make_plugin_allocator("debug"); // The allocator object is the memory of the actual allocator

		void *debug_memory = allocator_api->allocate(allocator_object, sizeof(Debug), __alignof(Debug));
		_debug_object = (new (debug_memory) Debug());

		_debug_object->allocator_api = allocator_api;
		_debug_object->allocator_object = allocator_object;

		return debug_memory;
	}

	void update(float dt) {
		_debug_object->update(dt);
	}

	void render() {
		_debug_object->render();
	}

	void shutdown() {
		AllocatorApi *allocator_api = _debug_object->allocator_api;
		AllocatorObject *allocator_object = _debug_object->allocator_object;

		_Application.release_world(_debug_object->world);

		_debug_object->~Debug();

		allocator_api->deallocate(allocator_object, _debug_object);
		allocator_api->destroy_plugin_allocator(allocator_object);
	}

	void on_script_reloaded(void *debug_memory) {
		_debug_object = (Debug*) debug_memory;
		_debug_object->clear();
	}

	#define _Debug (*debug::_debug_object)
	#define _DebugMenu (*debug::_debug_object).debug_menu

	#include "drawer.cpp"
}


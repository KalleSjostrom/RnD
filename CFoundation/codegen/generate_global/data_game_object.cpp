namespace settings {
	struct Data;
}

namespace game_object {
	struct Data {
		String name;
		unsigned name_id;
		settings::Data *settings;
	};
	#include "../utils/data_generic.inl"
}
typedef game_object::Data GameObject;
typedef game_object::Array GameObjectArray;
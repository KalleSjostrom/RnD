struct Settings;

struct GameObject {
	String name;
	unsigned name_id;
	Settings *settings;
};
typedef GameObject* GameObjectArray;
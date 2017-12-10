#pragma once

struct FieldChange {
	GameObjectId go_id;
	Id32 name;
	char *value;
};
__forceinline FieldChange make_field_change(GameObjectId go_id, Id32 name, char *value) {
	FieldChange field_change = { go_id, name, value };
	return field_change;
}

class GameObjectManager {
friend struct Reloader;

public:
	GameObjectManager();

	GameObjectId create_game_object(Id32 game_object_name, GameObjectField *default_fields, unsigned field_count);
	void add_game_object(GameObjectId go_id);
	void destroy_game_object(GameObjectId go_id);

	void read_game_object_fields();
	void read_game_object_fields_for(GameObjectId go_id);
	void push_field_change(GameObjectId go_id, Id32 name, char *value);
	void write_changed_fields();

	GameSessionPtr game_session;

private:
	// Buffer to hold changed game object fields. When this is full or when write_changed_fields() is called, we flush the changed fields to the engine.
	unsigned field_change_count;
	FieldChange field_changes[128];

	unsigned game_object_count;
	GameObjectId game_objects[MAX_INSTANCES];
};

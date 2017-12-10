#include "game_object_manager.h"

GameObjectManager::GameObjectManager() : game_session(0), field_change_count(0), game_object_count(0) {
}

GameObjectId GameObjectManager::create_game_object(Id32 game_object_name, GameObjectField *default_fields, unsigned field_count) {
	GameObjectId go_id = _GameSession.create_game_object(game_session, game_object_name, default_fields, field_count);
	add_game_object(go_id);
	return go_id;
}

void GameObjectManager::add_game_object(GameObjectId go_id) {
	ASSERT(game_object_count < MAX_INSTANCES, "Number of game objects exceeds MAX_INSTANCES");

	game_objects[game_object_count] = go_id;
	++game_object_count;
}

void GameObjectManager::destroy_game_object(GameObjectId go_id) {
	ASSERT(game_object_count > 0, "Number of game objects is less than 0?!");

	if (_GameSession.game_object_owner_is_self(game_session, go_id)) {
		_GameSession.destroy_game_object(game_session, go_id);
	}

	for (unsigned i = 0; i < game_object_count; i++) {
		if (game_objects[i] == go_id) {
			game_objects[i] = game_objects[--game_object_count];
			return;
		}
	}

	ASSERT(false, "Tried to destroy invalid game object.");
}

void GameObjectManager::read_game_object_fields() {
	Profile p("GameObjectManager::read_game_object_fields");
	for (unsigned i = 0; i < game_object_count; i++) {
		network_router::receive::read_game_object_fields(game_session, game_objects[i]);
	}
}
void GameObjectManager::read_game_object_fields_for(GameObjectId go_id) {
	network_router::receive::read_game_object_fields(game_session, go_id);
}

void GameObjectManager::push_field_change(GameObjectId go_id, Id32 name, char *value) {
	field_changes[field_change_count++] = make_field_change(go_id, name, value);

	// If we have more field changes than we have room, flush the buffer
	if (field_change_count == ARRAY_COUNT(field_changes))
		write_changed_fields();
}

void GameObjectManager::write_changed_fields() {
	Profile p("GameObjectManager::write_changed_fields");

	for (unsigned i = 0; i < field_change_count; i++) {
		FieldChange field_change = field_changes[i];
		_GameSession.set_game_object_field(game_session, field_change.go_id, field_change.name, field_change.value);
	}
	field_change_count = 0;
}

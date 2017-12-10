#include "component_manager.h"

ComponentManager::ComponentManager() :
			world(0), event_delegate(0), game_object_manager(0),
			deletion_locked(false),
			first_free_instance_index(0),
			marked_for_deletion_count(0) { }

ComponentManager::~ComponentManager() { }

void ComponentManager::init_components(WorldPtr world, CreationContext &context) {
	for (int i = 0; i < MAX_INSTANCES; ++i) {
		(instance_storage + i)->entity = INVALID_ENTITY;
	}
	hash_init(entity_instance_hashmap, entity_instance_hash_storage, ARRAY_COUNT(entity_instance_hash_storage), INVALID_ENTITY);
	hash_init(goid_instance_hashmap, goid_instance_hash_storage, ARRAY_COUNT(goid_instance_hash_storage), INVALID_GO_ID);

	components::manager = this;
	components::group = &component_group;

	this->world = world;
	this->event_delegate = context.event_delegate;
	this->game_object_manager = context.game_object_manager;
	component_group.init_components(context);
}

void ComponentManager::update(float dt) {
	Profile p("ComponentManager::update");

	if (game_object_manager)
		game_object_manager->read_game_object_fields();

	deletion_locked = true;

	marked_for_deletion = (EntityRef*)scratch_space::allocate(globals::scratch_space, MAX_INSTANCES * sizeof(EntityRef*));

	component_group.update(dt);

	if (game_object_manager)
		game_object_manager->write_changed_fields();

	deletion_locked = false;
	if (marked_for_deletion_count > 0) {
		for (unsigned i = 0; i < marked_for_deletion_count; ++i)
			remove(marked_for_deletion[i]);

		marked_for_deletion_count = 0;
	}
}

void ComponentManager::script_reload() {
	component_group.script_reload();
	components::manager = this;
	components::group = &component_group;
}

void ComponentManager::mark_for_deletion(EntityRef entity) {
	if (deletion_locked) {
		marked_for_deletion[marked_for_deletion_count++] = entity;
	} else {
		remove(entity);
	}
}

void ComponentManager::update_network_state(GameObjectId go_id, const char *buffer, unsigned *buffer_offset) {
	HashEntry *entry = hash_lookup(goid_instance_hashmap, go_id);
	ASSERT(entry->key == go_id, "Couldn't find entity with go_id %u", go_id);
	Instance *instance = instance_storage + entry->value;

	component_group.update_network_state(instance, buffer, buffer_offset);
}

void ComponentManager::game_object_created(int game_object_id) {
	GameSessionPtr game_session = game_object_manager->game_session;
	unsigned type_index = _GameSession.game_object_type(game_session, game_object_id);

	IdString64 entity_id = game_object_index_to_entity(type_index);
	SpawnContext sc = make_spawn_context(false);
	sc.go_id = game_object_id;
	game_object_manager->add_game_object(game_object_id);
	spawn_entity(entity_id, sc);
}

void ComponentManager::game_object_destroyed(int game_object_id) {
	GameObjectId go_id = game_object_id;
	Instance *instance = get_instance_from_goid(go_id);
	ASSERT(instance, "Game object destroyed, but I don't recognize the game object! (go_id=%u)", go_id);
	remove(instance->entity);
}

void ComponentManager::game_object_migrated_to_me(int game_object_id) {
	GameObjectId go_id = game_object_id;
	Instance *instance = get_instance_from_goid(go_id);
	ASSERT(instance, "Game object migrated to me, but I don't recognize the game object! (go_id=%u)", go_id);
	component_group.migrated_to_me(instance);
}

void ComponentManager::game_object_migrated_away(int game_object_id) {
	GameObjectId go_id = game_object_id;
	Instance *instance = get_instance_from_goid(go_id);
	ASSERT(instance, "Game object migrated away, but I don't recognize the game object! (go_id=%u)", go_id);
	component_group.migrated_away(instance);
}

void ComponentManager::update_first_free_index() {
	for (int i = 0; i < ARRAY_COUNT(instance_storage); ++i) {
		Instance *instance = instance_storage + first_free_instance_index;
		if (instance->entity == INVALID_ENTITY) {
			// this index is free! we are done
			return;
		}

		// Increment the first_free_instance_index
		if (first_free_instance_index == ARRAY_COUNT(instance_storage))
			first_free_instance_index = 0;
		else
			first_free_instance_index++;
	}

	ASSERT(false, "Too many entities registered in component manager!");
}

EntityRef ComponentManager::spawn_entity(IdString64 entity_path, SpawnContext &spawn_context) {
	EntityRef entity = _EntityManager.spawn(world, content::entities::dummy, spawn_context.pose);
	register_entity(entity, entity_path, spawn_context);
	return entity;
}

void ComponentManager::register_entity(EntityRef entity, IdString64 entity_id, SpawnContext &spawn_context) {
	// Get reference to the first free instance
	unsigned instance_index = first_free_instance_index;
	Instance *instance = &instance_storage[instance_index];

	// Reserve this instance
	hash_add(entity_instance_hashmap, entity, instance_index);

	// Fill in the instance with the entity information
	instance->entity = entity;
	instance->entity_id = entity_id;
	instance->unit = 0; // Is setup by the unit component
	instance->go_id = spawn_context.go_id;

	// Update the first free instance index to point to the next free instance
	update_first_free_index();

	if (spawn_context.component_spawn_context == 0)
		spawn_context.component_spawn_context = &empty_component_spawn_context;

	// Add this instance to the components it belongs to.
	component_group.add_to_component(instance, spawn_context, game_object_manager);

	// If we have a game object, insert it in the lookup
	if (instance->go_id != INVALID_GO_ID) {
		hash_add(goid_instance_hashmap, instance->go_id, instance_index);

		// If this is a remote instance, we need to read in the game object fields before notifying the components.
		if (!spawn_context.is_master)
			game_object_manager->read_game_object_fields_for(instance->go_id);
	}

	// Notify the components about the new instance by calling on_added() on each component.
	component_group.notify_on_added(instance, spawn_context);
}

void ComponentManager::remove(EntityRef entity) {
	ASSERT(!deletion_locked, "Trying to remove an entity while deletion is locked!");

	unsigned value = hash_remove(entity_instance_hashmap, entity);
	Instance *instance = instance_storage + value;

	event_delegate->trigger_on_entity_removed(entity);
	component_group.remove_from_component(instance);

	if (instance->go_id != INVALID_GO_ID) {
		game_object_manager->destroy_game_object(instance->go_id);

		hash_remove(goid_instance_hashmap, instance->go_id);
	}
	if (instance->unit) {
		_World.destroy_unit(world, instance->unit);
	}

	_EntityManager.destroy(instance->entity);

	instance->entity = INVALID_ENTITY; // Invalidate the entry
}

#pragma once

#define IS_MASTER(index) (index < master_count)
#define GOID_OWNER(go_id) (_GameSession.game_object_owner(_Network.game_session(), go_id))
#define GOID_OWNED(go_id) (_GameSession.game_object_owner_is_self(_Network.game_session(), go_id) > 0)

#define ENTITY_OWNER(entity) (GOID_OWNER(components::manager->get_go_id(entity)))
#define ENTITY_OWNED(entity) (GOID_OWNED(components::manager->get_go_id(entity)))

#define INPUT(entity, component, name, ...) (components::group->component.input_##name(entity, ##__VA_ARGS__))
#define COMMAND(entity, component, name, ...) (components::group->component.command_##name(entity, ##__VA_ARGS__))
#define COMPONENT_GET(entity, component, name) (components::group->component.get_##name(entity))

#define TRIGGER_EVENT(name, ...) (components::manager->event_delegate->trigger_##name(__VA_ARGS__))

#define REGISTER_ENTITY_EVENT(name, entity, object, receiver) (components::manager->event_delegate->register_##name(entity, object, ReceiverType_##receiver))
#define UNREGISTER_ENTITY_EVENT(name, entity, object) (components::manager->event_delegate->unregister_##name(entity, object))
#define TRIGGER_ENTITY_EVENT(name, entity, ...) (components::manager->event_delegate->trigger_##name(entity, ##__VA_ARGS__))

#define SET_GO_FIELD(index, go_field_name, go_field_value) do {\
	networks[index].go_field_name = (go_field_value);\
	components::manager->game_object_manager->push_field_change(instances[index]->go_id, ID(go_field_name), (char*)&(networks[index].go_field_name));\
} while(0,0)

#define SET_GO_FIELD_COMPONENT(component, index, go_field_name, go_field_value) do {\
	(component)->networks[index].go_field_name = (go_field_value);\
	components::manager->game_object_manager->push_field_change((component)->instances[index]->go_id, ID(go_field_name), (char*)&((component)->networks[index].go_field_name));\
} while(0,0)

#define HAS_COMPONENT(component, entity) (components::group->component.get_instance_id(entity) != INVALID_INSTANCE_ID)
#define GET_COMPONENT(component) (&components::group->component)
#define GET_SLAVE(component, entity) (GET_COMPONENT(component)->slaves[GET_COMPONENT(component)->get_instance_id(entity)])
#define GET_MASTER(component, entity) (GET_COMPONENT(component)->masters[GET_COMPONENT(component)->get_instance_id(entity)])
#define GET_NETWORK(component, entity) (GET_COMPONENT(component)->networks[GET_COMPONENT(component)->get_instance_id(entity)])

#define GO_ID_FROM_ENTITY(entity) (components::manager->get_go_id(entity))
#define ENTITY_FROM_GO_ID(go_id) (components::manager->try_get_entity(go_id))
#define INSTANCE_FROM_ENTITY(entity) (components::manager->get_instance_from_entity(entity))

#define MARK_FOR_DELETION(entity) (components::manager->mark_for_deletion(entity))

#include "generated/entity_lookup.generated.h"

struct Instance { // Per instance, but for all the component.
	EntityRef entity; // The glue that binds the entity data with the unit.
	IdString64 entity_id; // The id of this entity, use this in the entity_lookups to get e.g. game_object_type.
	UnitRef unit; // Backwards compatability with the unit system. This will be completly replaced by the entity.
	GameObjectId go_id; // The game object used to communicate over network.
};

// FULKOD(kalle): Generate these spawn contexts
struct EquipmentSpawnContext {
	EntityRef owner;
};
inline EquipmentSpawnContext make_equipment_spawn_context(EntityRef owner) {
	EquipmentSpawnContext sc = { owner };
	return sc;
}

struct UnitSpawnContext {
	unsigned level_index;
	unsigned index_in_level;
	bool is_level_unit;
};
inline UnitSpawnContext make_unit_spawn_context(unsigned level_index, unsigned index_in_level, bool is_level_unit) {
	UnitSpawnContext sc = { level_index, index_in_level, is_level_unit };
	return sc;
}

struct InventorySpawnContext {
	IdString64 primary_weapon;
	IdString64 support_weapon;
	IdString64 sidearm_weapon;
	IdString64 throwable_weapon;
};
inline InventorySpawnContext make_inventory_spawn_context(IdString64 primary_weapon, IdString64 support_weapon, IdString64 sidearm_weapon, IdString64 throwable_weapon) {
	InventorySpawnContext sc = { primary_weapon, support_weapon, sidearm_weapon, throwable_weapon };
	return sc;
}

struct SquadBrainSpawnContext {
	bool is_patrol;
	unsigned squad_type;
};
inline SquadBrainSpawnContext make_squad_brain_spawn_context(bool is_patrol, unsigned squad_type) {
	SquadBrainSpawnContext sbsc = { is_patrol, squad_type };
	return sbsc;
}

struct ComponentSpawnContext {
	EquipmentSpawnContext equipment_spawn_context;
	UnitSpawnContext unit_spawn_context;
	InventorySpawnContext inventory_spawn_context;
	SquadBrainSpawnContext squad_brain_spawn_context;
};

struct SpawnContext {
	bool is_master;
	GameObjectId go_id;
	const Matrix4x4 *pose;

	ComponentSpawnContext *component_spawn_context;
};
namespace {
	static const Matrix4x4 identity_matrix = matrix4x4_identity();
}
inline SpawnContext make_spawn_context(bool is_master, const Matrix4x4 *pose = 0, ComponentSpawnContext *component_spawn_context = 0) {
	SpawnContext sp = { is_master, INVALID_GO_ID, pose ? pose : &identity_matrix, component_spawn_context, };
	return sp;
}

class ComponentGroup;

struct CreationContext {
	EventDelegate *event_delegate;
	GameObjectManager *game_object_manager;
	CameraManager *camera_manager;
	ProjectileManager *projectile_manager;
	ExplosionManager *explosion_manager;
	GeneratedLevel *generated_level;
	NavWorld *nav_world;
	GuiPtr gui;
};

CreationContext make_creation_context(EventDelegate *event_delegate, GameObjectManager *game_object_manager, CameraManager *camera_manager, ProjectileManager *projectile_manager, ExplosionManager *explosion_manager, GeneratedLevel *generated_level, NavWorld *nav_world, GuiPtr gui) {
	CreationContext context = { event_delegate, game_object_manager, camera_manager, projectile_manager, explosion_manager, generated_level, nav_world, gui };
	return context;
}

#include "generated/components/component_group.generated.h"

class ComponentManager {
	friend struct Reloader;

public:
	ComponentManager();
	~ComponentManager();

	void init_components(WorldPtr world, CreationContext &context);

	void update(float dt);
	void script_reload();
	void mark_for_deletion(EntityRef entity);
	void update_network_state(GameObjectId go_id, const char *buffer, unsigned *buffer_offset);

	void game_object_created(int game_object_type);
	void game_object_destroyed(int game_object_id);
	void game_object_migrated_to_me(int game_object_id);
	void game_object_migrated_away(int game_object_id);

	EntityRef spawn_entity(IdString64 entity_path, SpawnContext &spawn_context);
	void register_entity(EntityRef entity, IdString64 entity_id, SpawnContext &spawn_context);
	void remove(EntityRef entity);

	__forceinline UnitRef get_unit(EntityRef entity) {
		HashEntry *entry = hash_lookup(entity_instance_hashmap, entity);
		ASSERT(entry->key == entity, "No such unit found!");
		return instance_storage[entry->value].unit;
	}
	__forceinline const GameObjectId *get_go_id_ptr(EntityRef entity) {
		if (entity == INVALID_ENTITY) { return &INVALID_GO_ID; }

		HashEntry *entry = hash_lookup(entity_instance_hashmap, entity);
		ASSERT(entry->key == entity, "No such game_object found!");
		return &instance_storage[entry->value].go_id;
	}
	__forceinline GameObjectId get_go_id(EntityRef entity) {
		return *get_go_id_ptr(entity);
	}
	__forceinline EntityRef get_entity(GameObjectId go_id) {
		HashEntry *entry = hash_lookup(goid_instance_hashmap, go_id);
		ASSERT(entry->key == go_id, "No such entity found!");
		return instance_storage[entry->value].entity;
	}
	__forceinline EntityRef try_get_entity(GameObjectId go_id) {
		if (go_id == INVALID_GO_ID) return INVALID_ENTITY;

		HashEntry *entry = hash_lookup(goid_instance_hashmap, go_id);
		if (entry->key == go_id)
			return instance_storage[entry->value].entity;
		else
			return INVALID_ENTITY;
	}
	__forceinline Instance *get_instance_from_goid(GameObjectId go_id) {
		HashEntry *entry = hash_lookup(goid_instance_hashmap, go_id);
		ASSERT(entry->key == go_id, "No such instance found!");
		return &instance_storage[entry->value];
	}
	__forceinline Instance *get_instance_from_entity(EntityRef entity) {
		HashEntry *entry = hash_lookup(entity_instance_hashmap, entity);
		ASSERT(entry->key == entity, "No such instance found!");
		return &instance_storage[entry->value];
	}

	// Stuff used by components but owned by StateGame
	WorldPtr world;
	EventDelegate *event_delegate;
	GameObjectManager *game_object_manager;

	ComponentGroup component_group;

private:
	bool deletion_locked;

	HashEntry entity_instance_hash_storage[HASH_SIZE_FOR(MAX_INSTANCES)];
	HashMap entity_instance_hashmap;

	HashEntry goid_instance_hash_storage[HASH_SIZE_FOR(MAX_INSTANCES)];
	HashMap goid_instance_hashmap;

	unsigned first_free_instance_index;
	Instance instance_storage[MAX_INSTANCES];

	unsigned marked_for_deletion_count;
	EntityRef *marked_for_deletion; // Allocated with temporary scratch space

	ComponentSpawnContext empty_component_spawn_context;

	void update_first_free_index();
};

namespace components {
	ComponentGroup *group;
	ComponentManager *manager;
}

#include "generated/components/component_group.generated.cpp"
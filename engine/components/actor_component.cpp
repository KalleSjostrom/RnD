#include "engine/utils/physics.cpp"

struct Actor {
	int tree_id;
};

struct ActorComponent {
	Actor actors[8];
	cid count;
	int tree_to_instance[MAX_AABB_TREE_NODES];
	AABBTree aabb_tree;
};

void update_pose(Matrix4x4 &pose, Vector3 *vertices, int vertex_count, AABB &aabb) {
	float minx = FLT_MAX;
	float maxx = -FLT_MAX;
	float miny = FLT_MAX;
	float maxy = -FLT_MAX;

	for (int i = 0; i < vertex_count; ++i) {
		Vector3 vertex = multiply_perspective(pose, vertices[i]);
		minx = fminf(vertex.x, minx);
		maxx = fmaxf(vertex.x, maxx);
		miny = fminf(vertex.y, miny);
		maxy = fmaxf(vertex.y, maxy);
	}

	aabb.x = minx;
	aabb.y = miny;
	aabb.w = maxx - minx;
	aabb.h = maxy - miny;
}

void set_pose(ActorComponent &ac, Entity &entity, Matrix4x4 &pose) {
	Actor &actor = ac.actors[entity.actor_id];
	AABB &aabb = ac.aabb_tree.aabb_storage[actor.tree_id];
	Shape &shape = ac.aabb_tree.shape_storage[actor.tree_id];

	update_pose(pose, shape.vertices, shape.vertex_count, aabb);

	ac.aabb_tree.remove_node(actor.tree_id);
	actor.tree_id = ac.aabb_tree.insert_node(aabb);

	ac.aabb_tree.shape_storage[actor.tree_id] = shape;
	ac.aabb_tree.pose_storage[actor.tree_id] = pose;
}

void add(ActorComponent &ac, Entity &entity, ShapeType type, Matrix4x4 &pose, Vector3 *vertices, int vertex_count) {
	ASSERT((u32)ac.count < ARRAY_COUNT(ac.actors), "Component full!");
	entity.actor_id = ac.count++;
	Actor &actor = ac.actors[entity.actor_id];

	// Add the aabb
	AABB aabb = {};
	update_pose(pose, vertices, vertex_count, aabb);
	actor.tree_id = ac.aabb_tree.insert_node(aabb);

	// Setup the shape
	Shape &shape = ac.aabb_tree.shape_storage[actor.tree_id];
	shape.type = type;
	shape.vertices = vertices;
	shape.vertex_count = vertex_count;

	ac.aabb_tree.pose_storage[actor.tree_id] = pose;

	// Save the mapping from component id to tree id.
	ac.tree_to_instance[actor.tree_id] = entity.actor_id;
}

inline OverlapResults overlap(ActorComponent &ac, Entity &entity) {
	Actor &actor = ac.actors[entity.actor_id];
	return overlap(ac.aabb_tree, actor.tree_id);
}

inline SweepResults sweep(ActorComponent &ac, Entity &entity, Vector3 &translation) {
	Actor &actor = ac.actors[entity.actor_id];
	return sweep(ac.aabb_tree, actor.tree_id, translation);
}

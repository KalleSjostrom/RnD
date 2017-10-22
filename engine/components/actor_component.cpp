#include "engine/utils/physics.cpp"

namespace actor_component {
	struct Instance {
		i32 tree_id;
	};

	struct ActorComponent {
		Instance instances[8];
		i32 count;
		i32 tree_to_instance[MAX_AABB_TREE_NODES];
		i32 __padding;
		i32 ___padding;
		i32 ____padding;
		AABBTree aabb_tree;

		void update_pose(m4 &pose, v3 *vertices, int32_t vertex_count, AABB &aabb) {
			float minx = FLT_MAX;
			float maxx = -FLT_MAX;
			float miny = FLT_MAX;
			float maxy = -FLT_MAX;

			for (i32 i = 0; i < vertex_count; ++i) {
				v3 vertex = multiply_perspective(pose, vertices[i]);
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

		void set_pose(i32 id, m4 &pose) {
			Instance &instance = instances[id];
			AABB &aabb = aabb_tree.aabb_storage[instance.tree_id];
			Shape &shape = aabb_tree.shape_storage[instance.tree_id];

			update_pose(pose, shape.vertices, shape.vertex_count, aabb);

			aabb_tree.remove_node(instance.tree_id);
			instance.tree_id = aabb_tree.insert_node(aabb);

			aabb_tree.shape_storage[instance.tree_id] = shape;
			aabb_tree.pose_storage[instance.tree_id] = pose;
		}

		i32 add(ShapeType type, m4 &pose, v3 *vertices, int32_t vertex_count) {
			ASSERT((u32)count < ARRAY_COUNT(instances), "Component full!");
			i32 id = count++;
			Instance &instance = instances[id];

			// Add the aabb
			AABB aabb = {};
			update_pose(pose, vertices, vertex_count, aabb);
			instance.tree_id = aabb_tree.insert_node(aabb);

			// Setup the shape
			Shape &shape = aabb_tree.shape_storage[instance.tree_id];
			shape.type = type;
			shape.vertices = vertices;
			shape.vertex_count = vertex_count;

			aabb_tree.pose_storage[instance.tree_id] = pose;

			// Save the mapping from component id to tree id.
			tree_to_instance[instance.tree_id] = id;

			return id;
		}

		inline RaycastResults raycast(Ray &ray) {
			return ::raycast(aabb_tree, ray);
		}

		inline OverlapResults overlap(i32 id) {
			Instance &instance = instances[id];
			return ::overlap(aabb_tree, instance.tree_id);
		}

		inline SweepResults sweep(i32 id, v3 &translation) {
			Instance &instance = instances[id];
			return ::sweep(aabb_tree, instance.tree_id, translation);
		}
	};
}

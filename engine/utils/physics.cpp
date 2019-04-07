struct AABB {
	float x, y; // Lower left corner
	float w, h;
	float operator[](int index) {
		return *((float*)this + index);
	}
};

__forceinline AABB make_aabb(float x, float y, float w, float h) {
	AABB result = {x, y, w, h};
	return result;
}
__forceinline AABB operator+(AABB &a, AABB &b) {
	float x = fminf(a.x, b.x);
	float y = fminf(a.y, b.y);
	float w = fmaxf(a.x+a.w, b.x+b.w) - x;
	float h = fmaxf(a.y+a.h, b.y+b.h) - y;
	return make_aabb(x, y, w, h);
}

__forceinline float area(AABB a) {
	return a.w * a.h;
}

__forceinline bool inside(AABB &a, Vector3 &p) {
	return (p.x > a.x) && (p.x < (a.x + a.w)) && (p.y > a.y) && (p.y < (a.y + a.h));
}

__forceinline bool are_overlapping(AABB &a, AABB &b) {
	return (a.x < b.x + b.w) && (a.x + a.w > b.x) && (a.y < b.y + b.h) && (a.h + a.y > b.y);
}

enum ShapeType {
	ShapeType_Box,
	ShapeType_AABox,
	ShapeType_Sphere,
	ShapeType_Polygon,
};

struct Shape {
	ShapeType type;

	int vertex_count;
	Vector3 *vertices;
};

struct Node {
	int left;
	int right;
	int parent;
};

#define NULL_ID 0
#define MAX_AABB_TREE_NODES 32

struct AABBTree {
	bool used_ids[MAX_AABB_TREE_NODES];
	AABB aabb_storage[MAX_AABB_TREE_NODES];
	Shape shape_storage[MAX_AABB_TREE_NODES];
	Matrix4x4 pose_storage[MAX_AABB_TREE_NODES];
	Node nodes[MAX_AABB_TREE_NODES];

	int cursor;
	int root;
	int64_t __padding;

	int new_id() {
		for (int i = 0; i < (int)ARRAY_COUNT(used_ids); ++i) {
			cursor++;
			if (cursor == ARRAY_COUNT(used_ids))
				cursor = 1;

			if (!used_ids[cursor]) {
				used_ids[cursor] = true;
				return cursor;
			}
		}
		ASSERT(false, "AABBTree full!");
		return NULL_ID;
	}

	void init_node(int id, int left = NULL_ID, int right = NULL_ID, int parent = NULL_ID) {
		Node *n = &nodes[id];
		n->left = left;
		n->right = right;
		n->parent = parent;
	}

	inline void deinit_node(Node &n) {
		n.left = n.right = n.parent = NULL_ID;
	}

	int insert_node(AABB &box) {
		int id = new_id();
		aabb_storage[id] = box;

		init_node(id);

		if (root == NULL_ID)
			root = id;
		else
			_insert_node(id, root);
		return id;
	}
	int _insert_node(int id, int parent) {
		ASSERT(parent, "");
		Node &p = nodes[parent];

		if (p.left == NULL_ID) { // We are a leaf
			int left = parent; // "Move" the parent down to the left child.
			int right = id; // Set the new entry as the right child
			// This is already reset from when creating node above.

			int new_parent = new_id(); // Create a new AABB for the parent...
			if (parent == root) { // Update root if needed.
				root = new_parent;
			}
			init_node(new_parent, left, right, p.parent);

			nodes[left].parent = new_parent;
			nodes[right].parent = new_parent;

			aabb_storage[new_parent] = aabb_storage[left] + aabb_storage[right]; // ... and fill it with the sum of the childrens.
			return new_parent;
		} else {
			int left = p.left;
			int right = p.right;

			// Insert id in the left or right sub tree based on which would cause the least increase in area.
			AABB &left_aabb = aabb_storage[left];
			AABB &right_aabb = aabb_storage[right];

			float area_delta_left = area(left_aabb + aabb_storage[id]) - area(left_aabb);
			float area_delta_right = area(right_aabb + aabb_storage[id]) - area(right_aabb);

			if (area_delta_left < area_delta_right) {
				p.left = _insert_node(id, left);
			} else {
				p.right = _insert_node(id, right);
			}
			aabb_storage[parent] = aabb_storage[p.left] + aabb_storage[p.right];
			return parent;
		}
	}

	// This function moves a sibling up the tree, replacing it's current parent.
	inline void promote_sibling(int sibling, int parent) {
		ASSERT(parent != NULL_ID, "Can't move sibling up the tree to a null node");

		Node &s = nodes[sibling];
		Node &p = nodes[parent];

		s.parent = p.parent;
		if (s.parent) { // If there is a grandparent, we need to patch it's child pointers to us
			Node &new_parent = nodes[s.parent];
			// Are we the left or the right side of the grandparents tree?
			if (new_parent.left == parent)
				new_parent.left = sibling;
			else
				new_parent.right = sibling;
		}
	}

	void remove_node(int id) {
		used_ids[id] = false;

		Node &n = nodes[id];
		ASSERT(n.left == NULL_ID, "Can't remove a non-leaf node!");

		int parent = n.parent;
		if (parent) {
			used_ids[parent] = false; // Remove parent, no need to have a compund node here. It will be replaced by the sibling instead.
			Node &p = nodes[parent];

			int sibling = p.left == id ? p.right : p.left; // Which side is the sibling?
			promote_sibling(sibling, parent);
			deinit_node(p); // Clear out the removed node

			if (parent == root) { // Update root if needed.
				root = sibling;
			}

			Node *node_cursor = nodes + sibling;
			while (node_cursor->parent) {
				Node &temp = nodes[node_cursor->parent];
				aabb_storage[node_cursor->parent] = aabb_storage[temp.left] + aabb_storage[temp.right];
				node_cursor = &temp;
			}
		}

		deinit_node(n);
	}
};

struct Ray {
	Vector3 from;
	Vector3 to;
	Vector3 delta;
	Vector2 inv_delta;
};
#include "math/intersection.cpp"

__forceinline Ray make_ray(Vector3 &from, Vector3 &to) {
	Ray r;

	r.from = from;
	r.to = to;

	r.delta = to - from;
	r.inv_delta.x = safe_divide(1.0f, r.delta.x, FLT_MAX);
	r.inv_delta.y = safe_divide(1.0f, r.delta.y, FLT_MAX);

	return r;
}

// TODO(kalle): Extend to return a list of results, the closest hit or just an arbitrary hit.
struct RaycastResults {
	int id;
	Vector3 position;
	Vector3 normal;
};
RaycastResults raycast(AABBTree &tree, Ray &ray) {
	RaycastResults results = {};

	int count = 0;
	int at = 0;
	int queue[32] = {};
	queue[count++] = tree.root;
	while (at < count) {
		int id = queue[at++];
		AABB &aabb = tree.aabb_storage[id];
		bool is_inside = inside(aabb, ray.from);
		bool is_hit = false;

		intersection::RayAABB ir = {};
		if (!is_inside) {
			ir = intersection::ray_aabb(ray, aabb);
			is_hit = ir.did_hit();
		}

		if (is_inside || is_hit) {
			Node &n = tree.nodes[id];
			if (n.left == NULL_ID) {
				if (is_hit)
				{ // raycast_narrowphase(AABBTree &tree, Ray &ray, RaycastResults &result, Shape &shape) {
					bool did_hit = false;

					Shape &shape = tree.shape_storage[id];
					switch (shape.type) {
						case ShapeType_Box: {
							// Vector3 direction = normalize(ray.delta);

							Matrix4x4 &pose = tree.pose_storage[id];

							Vector3 tv0 = multiply_perspective(pose, shape.vertices[0]);
							Vector3 tv1 = multiply_perspective(pose, shape.vertices[1]);
							Vector3 tv2 = multiply_perspective(pose, shape.vertices[2]);
							Vector3 tv3 = multiply_perspective(pose, shape.vertices[3]);

							intersection::RayRay closest_intersection = intersection::RayRay();
							intersection::ray_ray(tv0, tv1-tv0, ray.from, ray.delta, &closest_intersection);
							intersection::ray_ray(tv1, tv3-tv1, ray.from, ray.delta, &closest_intersection);
							intersection::ray_ray(tv3, tv2-tv3, ray.from, ray.delta, &closest_intersection);
							intersection::ray_ray(tv2, tv0-tv2, ray.from, ray.delta, &closest_intersection);

							did_hit = closest_intersection.did_hit();
							if (did_hit) {
								results.position = ray.from + ray.delta * closest_intersection.t;
								results.normal = closest_intersection.normal();
							}
						} break;
						case ShapeType_AABox: {
							did_hit = true;

							results.position = ray.from + ray.delta * ir.tmin;

							if (float_equal(ir.tx1, ir.tmin)) {
								results.normal = vector3(-1, 0, 0);
							} else if (float_equal(ir.tx2, ir.tmin)) {
								results.normal = vector3(1, 0, 0);
							} else if (float_equal(ir.ty1, ir.tmin)) {
								results.normal = vector3(0, -1, 0);
							} else if (float_equal(ir.ty2, ir.tmin)) {
								results.normal = vector3(0, 1, 0);
							}
						} break;
						case ShapeType_Sphere: {
							float radius = 50;

							Matrix4x4 &pose = tree.pose_storage[id];
							Vector3 center = translation(pose);

							Vector3 to_center = center - ray.from;
							Vector3 delta_n = normalize(ray.delta);
							float length_along_delta = dot(to_center, delta_n);

							Vector3 closest_point = ray.from + length_along_delta * delta_n;
							float opposite_cathetus = length(center - closest_point);
							if (opposite_cathetus < radius) {
								float hypotenuse = radius;
								float cathetus = sqrtf(hypotenuse * hypotenuse - opposite_cathetus * opposite_cathetus);

								did_hit = true;
								results.position = closest_point - delta_n * cathetus;
								results.normal = normalize(results.position - center);
							}
						} break;
						case ShapeType_Polygon: {

						} break;
					}
					if (did_hit) {
						results.id = id;
						break;
					}
				}
			} else {
				queue[count++] = n.left;
				queue[count++] = n.right;
			}
		}
	}

	return results;
}

struct OverlapResults {
	int id;
	Vector3 position;
	Vector3 normal;
};
OverlapResults overlap(AABBTree &tree, int item) {
	OverlapResults results = {};

	AABB &aabb = tree.aabb_storage[item];

	int count = 0;
	int at = 0;
	int queue[32] = {};
	queue[count++] = tree.root;
	while (at < count) {
		int id = queue[at++];
		if (id == item)
			continue;

		if (are_overlapping(tree.aabb_storage[id], aabb)) {
			Node &n = tree.nodes[id];
			if (n.left != NULL_ID) { // It has children, keep digging down
				queue[count++] = n.left;
				queue[count++] = n.right;
			} else {
				bool did_hit = true;
				Shape &shape = tree.shape_storage[id];
				switch (shape.type) {
					// TODO(kalle): Implement narrowphase
					case ShapeType_Box: {
					} break;
					case ShapeType_AABox: {
					} break;
					case ShapeType_Sphere: {
					} break;
					case ShapeType_Polygon: {
					} break;
				}
				if (did_hit) {
					results.id = id;
					break;
				}
			}
		}
	}

	return results;
}


struct SweepResults {
	int id;
	Vector3 constrained_translation;
	Vector3 normal;
};
SweepResults sweep(AABBTree &tree, int item, Vector3 &wanted_translation) {
	SweepResults results = {};

	results.constrained_translation = wanted_translation;

	AABB &from = tree.aabb_storage[item];
	AABB to = from;
	to.x += wanted_translation.x;
	to.y += wanted_translation.y;

	AABB aabb = from + to;

	int count = 0;
	int at = 0;
	int queue[32] = {};
	queue[count++] = tree.root;
	while (at < count) {
		int id = queue[at++];
		if (id == item)
			continue;

		AABB &other = tree.aabb_storage[id];
		if (are_overlapping(other, aabb)) {
			Node &n = tree.nodes[id];
			if (n.left != NULL_ID) { // It has children, keep digging down
				queue[count++] = n.left;
				queue[count++] = n.right;
			} else {
				bool did_hit = false;
				Shape &shape = tree.shape_storage[id];
				switch (shape.type) {
					case ShapeType_Box: {
						intersection::AabbAabb out;
						did_hit = intersection::moving_aabb_aabb(from, other, wanted_translation, &out);
						if (did_hit) {
							results.constrained_translation.y = (wanted_translation * out.t).y;
						}
					} break;
					case ShapeType_AABox: {
						intersection::AabbAabb out;
						did_hit = intersection::moving_aabb_aabb(from, other, wanted_translation, &out);
						if (did_hit) {
							results.constrained_translation.y = (wanted_translation * out.t).y;
						}
					} break;
					case ShapeType_Sphere: {
					} break;
					case ShapeType_Polygon: {
					} break;
				}
				if (did_hit) {
					results.id = id;
					break;
				}
			}
		}
	}

	return results;
}

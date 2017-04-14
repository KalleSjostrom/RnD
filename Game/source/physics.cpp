struct AABB {
	float x, y;
	float w, h;
};

__forceinline AABB make_aabb(float x, float y, float w, float h) {
	AABB result = {x, y, w, h};
	return result;
}
__forceinline AABB operator+(AABB &a, AABB &b) {
	float x = fmin(a.x, b.x);
	float y = fmin(a.y, b.y);
	float w = fmax(a.x+a.w, b.x+b.w) - x;
	float h = fmax(a.y+a.h, b.y+b.h) - y;
	return make_aabb(x, y, w, h);
}

__forceinline float area(AABB a) {
	return a.w * a.h;
}

__forceinline bool inside(AABB &a, v3 &p) {
	return (p.x > a.x) && (p.x < (a.x + a.w)) && (p.y > a.y) && (p.y < (a.y + a.h));
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
	v3 *vertices;
};

struct Node {
	int left;
	int right;
	int parent;
};

#define NULL_ID 0
#define MAX_AABB_TREE_NODES 32

struct AABBTree {
	int cursor;
	bool used_ids[MAX_AABB_TREE_NODES];
	AABB aabb_storage[MAX_AABB_TREE_NODES];
	Shape shape_storage[MAX_AABB_TREE_NODES];
	m4 pose_storage[MAX_AABB_TREE_NODES];

	Node nodes[MAX_AABB_TREE_NODES];
	int root;

	int new_id() {
		for (int i = 0; i < ARRAY_COUNT(used_ids); ++i) {
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
	};

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
	v3 from;
	v3 to;
	v3 delta;
	v2 inv_delta;
};
#include "intersection.cpp"

__forceinline Ray make_ray(v3 &from, v3 &to) {
	Ray r;

	r.from = from;
	r.to = to;

	r.delta = to - from;
	r.inv_delta.x = safe_divide(1.0f, r.delta.x, INFINITY);
	r.inv_delta.y = safe_divide(1.0f, r.delta.y, INFINITY);

	return r;
}

struct RaycastResults {
	int id;
	v3 position;
	v3 normal;
};


// TODO(kalle): Extend to return a list of results, the closest hit or just an arbitrary hit.
RaycastResults raycast(AABBTree &tree, Ray &ray) {
	Node &root = tree.nodes[tree.root];
	RaycastResults rr = {};

	int count = 0;
	int at = 0;
	int queue[32] = {};
	queue[count++] = tree.root;
	while (at < count) {
		int id = queue[at++];
		AABB &aabb = tree.aabb_storage[id];
		bool is_inside = inside(aabb, ray.from);
		bool is_hit = false;

		intersection::RayAABB ir;
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
							v3 direction = normalize(ray.delta);

							m4 &pose = tree.pose_storage[id];

							v3 tv0 = multiply_perspective(pose, shape.vertices[0]);
							v3 tv1 = multiply_perspective(pose, shape.vertices[1]);
							v3 tv2 = multiply_perspective(pose, shape.vertices[2]);
							v3 tv3 = multiply_perspective(pose, shape.vertices[3]);

							intersection::RayRay closest_intersection = intersection::RayRay();
							intersection::ray_ray(tv0, tv1-tv0, ray.from, ray.delta, &closest_intersection);
							intersection::ray_ray(tv1, tv3-tv1, ray.from, ray.delta, &closest_intersection);
							intersection::ray_ray(tv3, tv2-tv3, ray.from, ray.delta, &closest_intersection);
							intersection::ray_ray(tv2, tv0-tv2, ray.from, ray.delta, &closest_intersection);

							did_hit = closest_intersection.did_hit();
							if (did_hit) {
								rr.position = ray.from + ray.delta * closest_intersection.t;
								rr.normal = closest_intersection.normal();
							}
						} break;
						case ShapeType_AABox: {
							did_hit = true;

							rr.position = ray.from + ray.delta * ir.tmin;

							if (ir.tx1 == ir.tmin) {
								rr.normal = V3(-1, 0, 0);
							} else if (ir.tx2 == ir.tmin) {
								rr.normal = V3(1, 0, 0);
							} else if (ir.ty1 == ir.tmin) {
								rr.normal = V3(0, -1, 0);
							} else if (ir.ty2 == ir.tmin) {
								rr.normal = V3(0, 1, 0);
							}
						} break;
						case ShapeType_Sphere: {
							float radius = 50;

							m4 &pose = tree.pose_storage[id];
							v3 center = translation(pose);

							v3 to_center = center - ray.from;
							v3 delta_n = normalize(ray.delta);
							float length_along_delta = dot(to_center, delta_n);

							v3 closest_point = ray.from + length_along_delta * delta_n;
							float opposite_cathetus = length(center - closest_point);
							if (opposite_cathetus < radius) {
								float hypotenuse = radius;
								float cathetus = sqrtf(hypotenuse * hypotenuse - opposite_cathetus * opposite_cathetus);

								did_hit = true;
								rr.position = closest_point - delta_n * cathetus;
								rr.normal = normalize(rr.position - center);
							}
						} break;
						case ShapeType_Polygon: {

						} break;
					}
					if (did_hit) {
						rr.id = id;
						break;
					}
				}
			} else {
				queue[count++] = n.left;
				queue[count++] = n.right;
			}
		}
	}

	return rr;
}

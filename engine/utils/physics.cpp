struct AABB {
	f32 x, y; // Lower left corner
	f32 w, h;
	float operator[](int index) {
		return *((float*)this + index);
	}
};

FORCE_INLINE AABB make_aabb(f32 x, f32 y, f32 w, f32 h) {
	AABB result = {x, y, w, h};
	return result;
}
FORCE_INLINE AABB operator+(AABB &a, AABB &b) {
	f32 x = fminf(a.x, b.x);
	f32 y = fminf(a.y, b.y);
	f32 w = fmaxf(a.x+a.w, b.x+b.w) - x;
	f32 h = fmaxf(a.y+a.h, b.y+b.h) - y;
	return make_aabb(x, y, w, h);
}

FORCE_INLINE f32 area(AABB a) {
	return a.w * a.h;
}

FORCE_INLINE bool inside(AABB &a, v3 &p) {
	return (p.x > a.x) && (p.x < (a.x + a.w)) && (p.y > a.y) && (p.y < (a.y + a.h));
}

FORCE_INLINE bool are_overlapping(AABB &a, AABB &b) {
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

	i32 vertex_count;
	v3 *vertices;
};

struct Node {
	i32 left;
	i32 right;
	i32 parent;
};

#define NULL_ID 0
#define MAX_AABB_TREE_NODES 32

struct AABBTree {
	bool used_ids[MAX_AABB_TREE_NODES];
	AABB aabb_storage[MAX_AABB_TREE_NODES];
	Shape shape_storage[MAX_AABB_TREE_NODES];
	m4 pose_storage[MAX_AABB_TREE_NODES];
	Node nodes[MAX_AABB_TREE_NODES];

	i32 cursor;
	i32 root;
	i64 __padding;

	i32 new_id() {
		for (i32 i = 0; i < (i32)ARRAY_COUNT(used_ids); ++i) {
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

	void init_node(i32 id, i32 left = NULL_ID, i32 right = NULL_ID, i32 parent = NULL_ID) {
		Node *n = &nodes[id];
		n->left = left;
		n->right = right;
		n->parent = parent;
	}

	inline void deinit_node(Node &n) {
		n.left = n.right = n.parent = NULL_ID;
	}

	i32 insert_node(AABB &box) {
		i32 id = new_id();
		aabb_storage[id] = box;

		init_node(id);

		if (root == NULL_ID)
			root = id;
		else
			_insert_node(id, root);
		return id;
	}
	i32 _insert_node(i32 id, i32 parent) {
		ASSERT(parent, "");
		Node &p = nodes[parent];

		if (p.left == NULL_ID) { // We are a leaf
			i32 left = parent; // "Move" the parent down to the left child.
			i32 right = id; // Set the new entry as the right child
			// This is already reset from when creating node above.

			i32 new_parent = new_id(); // Create a new AABB for the parent...
			if (parent == root) { // Update root if needed.
				root = new_parent;
			}
			init_node(new_parent, left, right, p.parent);

			nodes[left].parent = new_parent;
			nodes[right].parent = new_parent;

			aabb_storage[new_parent] = aabb_storage[left] + aabb_storage[right]; // ... and fill it with the sum of the childrens.
			return new_parent;
		} else {
			i32 left = p.left;
			i32 right = p.right;

			// Insert id in the left or right sub tree based on which would cause the least increase in area.
			AABB &left_aabb = aabb_storage[left];
			AABB &right_aabb = aabb_storage[right];

			f32 area_delta_left = area(left_aabb + aabb_storage[id]) - area(left_aabb);
			f32 area_delta_right = area(right_aabb + aabb_storage[id]) - area(right_aabb);

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
	inline void promote_sibling(i32 sibling, i32 parent) {
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

	void remove_node(i32 id) {
		used_ids[id] = false;

		Node &n = nodes[id];
		ASSERT(n.left == NULL_ID, "Can't remove a non-leaf node!");

		i32 parent = n.parent;
		if (parent) {
			used_ids[parent] = false; // Remove parent, no need to have a compund node here. It will be replaced by the sibling instead.
			Node &p = nodes[parent];

			i32 sibling = p.left == id ? p.right : p.left; // Which side is the sibling?
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
#include "math/intersection.cpp"

FORCE_INLINE Ray make_ray(v3 &from, v3 &to) {
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
	i32 id;
	v3 position;
	v3 normal;
};
RaycastResults raycast(AABBTree &tree, Ray &ray) {
	RaycastResults results = {};

	i32 count = 0;
	i32 at = 0;
	i32 queue[32] = {};
	queue[count++] = tree.root;
	while (at < count) {
		i32 id = queue[at++];
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
							// v3 direction = normalize(ray.delta);

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
								results.position = ray.from + ray.delta * closest_intersection.t;
								results.normal = closest_intersection.normal();
							}
						} break;
						case ShapeType_AABox: {
							did_hit = true;

							results.position = ray.from + ray.delta * ir.tmin;

							if (float_equal(ir.tx1, ir.tmin)) {
								results.normal = V3(-1, 0, 0);
							} else if (float_equal(ir.tx2, ir.tmin)) {
								results.normal = V3(1, 0, 0);
							} else if (float_equal(ir.ty1, ir.tmin)) {
								results.normal = V3(0, -1, 0);
							} else if (float_equal(ir.ty2, ir.tmin)) {
								results.normal = V3(0, 1, 0);
							}
						} break;
						case ShapeType_Sphere: {
							f32 radius = 50;

							m4 &pose = tree.pose_storage[id];
							v3 center = translation(pose);

							v3 to_center = center - ray.from;
							v3 delta_n = normalize(ray.delta);
							f32 length_along_delta = dot(to_center, delta_n);

							v3 closest_point = ray.from + length_along_delta * delta_n;
							f32 opposite_cathetus = length(center - closest_point);
							if (opposite_cathetus < radius) {
								f32 hypotenuse = radius;
								f32 cathetus = sqrtf(hypotenuse * hypotenuse - opposite_cathetus * opposite_cathetus);

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
	i32 id;
	v3 position;
	v3 normal;
};
OverlapResults overlap(AABBTree &tree, i32 item) {
	OverlapResults results = {};

	AABB &aabb = tree.aabb_storage[item];

	i32 count = 0;
	i32 at = 0;
	i32 queue[32] = {};
	queue[count++] = tree.root;
	while (at < count) {
		i32 id = queue[at++];
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
	i32 id;
	v3 constrained_translation;
	v3 normal;
};
SweepResults sweep(AABBTree &tree, i32 item, v3 &wanted_translation) {
	SweepResults results = {};

	results.constrained_translation = wanted_translation;

	AABB &from = tree.aabb_storage[item];
	AABB to = from;
	to.x += wanted_translation.x;
	to.y += wanted_translation.y;

	AABB aabb = from + to;

	i32 count = 0;
	i32 at = 0;
	i32 queue[32] = {};
	queue[count++] = tree.root;
	while (at < count) {
		i32 id = queue[at++];
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

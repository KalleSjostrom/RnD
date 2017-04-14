namespace intersection {
	struct RayAABB {
		float tx1, tx2;
		float ty1, ty2;
		float tmin, tmax;
		bool did_hit() {
			return tmax > tmin && tmin >= 0.0f && tmin <= 1.0f;
		}
	};

	/* Slab method
	p_x + t_xmin * dx = x_min
	solve for t_xmin:
	t_xmin = (x_min - p_x) / dx; */
	static const RayAABB ray_aabb(Ray &ray, AABB &aabb) {
		RayAABB ir;

		// tx1 is normalized distance along the ray to the vertical line through aabb.x
		ir.tx1 = (aabb.x - ray.from.x) * ray.inv_delta.x;
		ir.tx2 = ((aabb.x + aabb.w) - ray.from.x) * ray.inv_delta.x;

		ir.ty1 = (aabb.y - ray.from.y) * ray.inv_delta.y;
		ir.ty2 = ((aabb.y + aabb.h) - ray.from.y) * ray.inv_delta.y;

		ir.tmin = fmax(fmin(ir.tx1, ir.tx2), fmin(ir.ty1, ir.ty2));
		ir.tmax = fmin(fmax(ir.tx1, ir.tx2), fmax(ir.ty1, ir.ty2));

		return ir;
	}

	struct RayRay {
		float t;
		float nx, ny;
		RayRay() : t(10), nx(0), ny(0) {}
		v3 normal() {
			return normalize(V3(nx, ny, 0));
		}
		bool did_hit() {
			return t >= 0.0f && t <= 1.0f;
		}
	};

	/*
	Given two rays with starting points p1 and p2 as well as direction vectors d1, d2 (not necessarily normalized).
	The two lines intersect if there is an intersection point p:
	p = p1 + d1 * u
	p = p2 + d2 * v

	If this equation system has a solution for u >= 0 and v >= 0, the rays intersect.

	p1.x + d1.x * u = p2.x + d2.x * v
	p1.y + d1.y * u = p2.y + d2.y * v

	solving for u:
	u = (p2.x + d2.x * v - p1.x) / d1.x
	u = (p2.y + d2.y * v - p1.y) / d1.y

	u = (p2.x + d2.x * v - p1.x) / d1.x = (p2.y + d2.y * v - p1.y) / d1.y

	isolate v terms:
	p2.x * d1.y + d2.x * v * d1.y - p1.x * d1.y = p2.y * d1.x + d2.y * v * d1.x - p1.y * d1.x
	p2.x * d1.y - p1.x * d1.y - p2.y * d1.x + p1.y * d1.x = (d2.y * d1.x - d2.x * d1.y) * v
	(p2.x * d1.y - p1.x * d1.y - p2.y * d1.x + p1.y * d1.x) / (d2.y * d1.x - d2.x * d1.y) = v
	*/
	void ray_ray(const v3 &p1, const v3 &d1, const v3 &p2, const v3 &d2, RayRay *out) {
		float denominator = d2.y * d1.x - d2.x * d1.y;
		if (fabs(denominator) > 0.0001f) {
			float nominator = p2.x * d1.y - p1.x * d1.y - p2.y * d1.x + p1.y * d1.x;
			float v = nominator / denominator;

			if (v >= 0 && v <= 1 && v < out->t) {
				float u;
				if (fabs(d1.x) > fabs(d1.y)) {
					u = (p2.x + d2.x * v - p1.x) / d1.x;
				} else {
					u = (p2.y + d2.y * v - p1.y) / d1.y;
				}

				if (u >= 0 && u <= 1) {
					out->t = v;
					out->nx = -d1.y;
					out->ny = d1.x;
				}
			}
		}
	}
}
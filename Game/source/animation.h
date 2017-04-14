#include "../generated/animations.generated.cpp"

struct Animation {
	float time;
	animation::Data *data;
};

struct Skeleton {
	int count;
	v3 position[4];
};

static GLindex skeleton_default_indices[] = { 0, 1, 1, 2, 0, 3 };

void skeleton_get_vertices(Skeleton &skeleton, v3 *vertices) {
	for (int i = 0; i < skeleton.count; ++i) {
		vertices[i] = skeleton.position[i] * 10.0f;
	}
}

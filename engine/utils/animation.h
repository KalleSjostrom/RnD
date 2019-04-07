
struct AnimationData {
	Vector3 *keyframes;
	int keyframe_count;
	int stride;
};
inline AnimationData make_animation_data(Vector3 *keyframes, int keyframe_count, int stride) {
	AnimationData d = { keyframes, keyframe_count, stride };
	return d;
}

struct Skeleton {
	int count;
	Vector3 position[4];
};

static GLindex skeleton_default_indices[] = { 0, 1, 1, 2, 0, 3 };

void skeleton_get_vertices(Skeleton &skeleton, Vector3 *vertices) {
	for (int i = 0; i < skeleton.count; ++i) {
		vertices[i] = skeleton.position[i] * 10.0f;
	}
}


struct AnimationData {
	v3 *keyframes;
	i32 keyframe_count;
	i32 stride;
};
inline AnimationData make_animation_data(v3 *keyframes, i32 keyframe_count, i32 stride) {
	AnimationData d = { keyframes, keyframe_count, stride };
	return d;
}

struct Skeleton {
	i32 count;
	v3 position[4];
};

static GLindex skeleton_default_indices[] = { 0, 1, 1, 2, 0, 3 };

void skeleton_get_vertices(Skeleton &skeleton, v3 *vertices) {
	for (i32 i = 0; i < skeleton.count; ++i) {
		vertices[i] = skeleton.position[i] * 10.0f;
	}
}

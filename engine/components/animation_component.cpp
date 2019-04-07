#include "engine/utils/animation.h"

#define ANIM(name) &animation::name::data

static f32 frames_per_second = 30.0f;

struct Animation {
	Skeleton skeleton;
	AnimationData *data;
	f32 time;
};

struct AnimationComponent {
	Animation animations[8];
	cid count;
};

void set_animation(AnimationComponent &ac, Entity &entity, AnimationData *data) {
	Animation &animation = ac.animations[entity.animation_id];

	animation.time = 0;
	animation.data = data;

	int c = data->keyframe_count / data->stride;
	for (int i = 0; i < c; ++i) {
		int frame = i * data->stride;
		animation.skeleton.position[i] = data->keyframes[frame];
	}
}

void add(AnimationComponent &ac, Entity &entity, AnimationData *data) {
	ASSERT((u32)ac.count < ARRAY_COUNT(ac.animations), "Component full!");
	entity.animation_id = ac.count++;
	Animation &animation = ac.animations[entity.animation_id];

	animation.skeleton.count = ARRAY_COUNT(animation.skeleton.position);

	set_animation(ac, entity, data);
}

void get_skeleton_vertices(AnimationComponent &ac, Entity &entity, Vector3 *vertices) {
	Animation &animation = ac.animations[entity.animation_id];
	skeleton_get_vertices(animation.skeleton, vertices);
}

int get_skeleton_vertex_count(AnimationComponent &ac, Entity &entity) {
	Animation &animation = ac.animations[entity.animation_id];
	return animation.skeleton.count;
}


inline int _get_frame(Animation &animation) {
	return (int)(animation.time * frames_per_second);
}

void update(AnimationComponent &ac, f32 dt) {
	for (int i = 0; i < ac.count; ++i) {
		Animation &animation = ac.animations[i];
		AnimationData *data = animation.data;

		animation.time += dt;
		int frame = _get_frame(animation);
		f32 start_time = frame / frames_per_second;
		// f32 end_time = (frame+1) / frames_per_second;

		f32 t = (animation.time - start_time) * frames_per_second;
		if (frame >= data->stride) {
			animation.time -= data->stride / frames_per_second;
			frame = _get_frame(animation);
		}

		ASSERT(frame < data->stride, "Index out of bounds!");

		int frame_count = data->keyframe_count / data->stride;
		for (int j = 0; j < frame_count; ++j) {
			int start_frame = j * data->stride;
			// printf("%d\n", start_frame);
			Vector3 curr = data->keyframes[start_frame + frame];
			Vector3 next = data->keyframes[start_frame + ((frame == data->stride-1) ? 0:frame+1)];

			animation.skeleton.position[j] = lerp(curr, next, t);
		}
	}
}

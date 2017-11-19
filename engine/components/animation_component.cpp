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

	cid add(AnimationData *data) {
		ASSERT((u32)count < ARRAY_COUNT(animations), "Component full!");
		cid id = count++;
		Animation &animation = animations[id];

		animation.skeleton.count = ARRAY_COUNT(animation.skeleton.position);

		set_animation(id, data);

		return id;
	}

	void set_animation(i32 id, AnimationData *data) {
		Animation &animation = animations[id];

		animation.time = 0;
		animation.data = data;

		i32 c = data->keyframe_count / data->stride;
		for (i32 i = 0; i < c; ++i) {
			i32 frame = i * data->stride;
			animation.skeleton.position[i] = data->keyframes[frame];
		}
	}

	v3 *get_skeleton_vertices(i32 id) {
		Animation &animation = animations[id];
		v3 *vertices = SCRATCH_ALLOCATE_STRUCT(v3, (u32)animation.skeleton.count);
		skeleton_get_vertices(animation.skeleton, vertices);
		return vertices;
	}

	i32 get_skeleton_vertex_count(i32 id) {
		Animation &animation = animations[id];
		return animation.skeleton.count;
	}


	inline i32 _get_frame(Animation &animation) {
		return (i32)(animation.time * frames_per_second);
	}

	void update(f32 dt) {
		for (i32 i = 0; i < count; ++i) {
			Animation &animation = animations[i];
			AnimationData *data = animation.data;

			animation.time += dt;
			i32 frame = _get_frame(animation);
			f32 start_time = frame / frames_per_second;
			// f32 end_time = (frame+1) / frames_per_second;

			f32 t = (animation.time - start_time) * frames_per_second;
			if (frame >= data->stride) {
				animation.time -= data->stride / frames_per_second;
				frame = _get_frame(animation);
			}

			ASSERT(frame < data->stride, "Index out of bounds!");

			i32 frame_count = data->keyframe_count / data->stride;
			for (i32 j = 0; j < frame_count; ++j) {
				i32 start_frame = j * data->stride;
				// printf("%d\n", start_frame);
				v3 curr = data->keyframes[start_frame + frame];
				v3 next = data->keyframes[start_frame + ((frame == data->stride-1) ? 0:frame+1)];

				animation.skeleton.position[j] = lerp(curr, next, t);
			}
		}

		// for (i32 i = 0; i < count; ++i) {
		// 	Animation &animation = animations[i];
		// 	v3 *vertices = SCRATCH_ALLOCATE(v3, animation.skeleton.count);
		// 	skeleton_get_vertices(animation.skeleton, vertices);
		// 	CALL(entity, model, update_vertices, vertices);
		// }
	}
};

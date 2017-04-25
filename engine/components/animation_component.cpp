#define ANIM(name) &animation::name::data

static f32 frames_per_second = 30.0f;

namespace animation_component {
	struct Instance {
		Skeleton skeleton;
		i32 __padding;

		Animation animation;
	};

	struct AnimationComponent {
		Instance instances[8];
		i32 count;
		i32 __padding;
		i64 ___padding;

		i32 add(AnimationData *data) {
			ASSERT((u32)count < ARRAY_COUNT(instances), "Component full!");
			i32 id = count++;
			Instance &instance = instances[id];

			instance.skeleton.count = ARRAY_COUNT(instance.skeleton.position);

			set_animation(id, data);

			return id;
		}

		void set_animation(i32 id, AnimationData *data) {
			Instance &instance = instances[id];

			instance.animation.time = 0;
			instance.animation.data = data;

			i32 c = data->keyframe_count / data->stride;
			for (i32 i = 0; i < c; ++i) {
				i32 frame = i * data->stride;
				instance.skeleton.position[i] = data->keyframes[frame];
			}
		}

		v3 *get_skeleton_vertices(i32 id) {
			Instance &instance = instances[id];
			v3 *vertices = SCRATCH_ALLOCATE_STRUCT(v3, (u32)instance.skeleton.count);
			skeleton_get_vertices(instance.skeleton, vertices);
			return vertices;
		}

		i32 get_skeleton_vertex_count(i32 id) {
			Instance &instance = instances[id];
			return instance.skeleton.count;
		}


		inline i32 _get_frame(Animation &animation) {
			return (i32)(animation.time * frames_per_second);
		}

		void update(f32 dt) {
			for (i32 i = 0; i < count; ++i) {
				Instance &instance = instances[i];
				Animation &animation = instance.animation;
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

					instance.skeleton.position[j] = lerp(curr, next, t);
				}
			}

			// for (i32 i = 0; i < count; ++i) {
			// 	Instance &instance = instances[i];
			// 	v3 *vertices = SCRATCH_ALLOCATE(v3, instance.skeleton.count);
			// 	skeleton_get_vertices(instance.skeleton, vertices);
			// 	CALL(entity, model, update_vertices, vertices);
			// }
		}
	};
}

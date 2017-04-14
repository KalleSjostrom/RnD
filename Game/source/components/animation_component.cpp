#define ANIM(name) &animation::name::data

static float frames_per_second = 30.0f;

namespace animation_component {
	struct Instance {
		Skeleton skeleton;
		Animation animation;
	};

	struct AnimationComponent {
		int count;
		Instance instances[8];

		int add(animation::Data *data) {
			ASSERT(count < ARRAY_COUNT(instances), "Component full!");
			int id = count++;
			Instance &instance = instances[id];

			instance.skeleton.count = ARRAY_COUNT(instance.skeleton.position);

			set_animation(id, data);

			return id;
		}

		void set_animation(int id, animation::Data *data) {
			Instance &instance = instances[id];

			instance.animation.time = 0;
			instance.animation.data = data;

			int count = data->keyframe_count / data->stride;
			for (int i = 0; i < count; ++i) {
				int frame = i * data->stride;
				instance.skeleton.position[i] = data->keyframes[frame];
			}
		}

		v3 *get_skeleton_vertices(int id) {
			Instance &instance = instances[id];
			v3 *vertices = SCRATCH_ALLOCATE_STRUCT(v3, instance.skeleton.count);
			skeleton_get_vertices(instance.skeleton, vertices);
			return vertices;
		}

		int get_skeleton_vertex_count(int id) {
			Instance &instance = instances[id];
			return instance.skeleton.count;
		}


		inline int _get_frame(Animation &animation) {
			return (int)(animation.time * frames_per_second);
		}

		void update(float dt) {
			for (int i = 0; i < count; ++i) {
				Instance &instance = instances[i];
				Animation &animation = instance.animation;
				animation::Data *data = animation.data;

				animation.time += dt;
				int frame = _get_frame(animation);
				float start_time = frame / frames_per_second;
				float end_time = (frame+1) / frames_per_second;

				float t = (animation.time - start_time) * frames_per_second;
				if (frame >= data->stride) {
					animation.time -= data->stride / frames_per_second;
					frame = _get_frame(animation);
				}

				ASSERT(frame < data->stride, "Index out of bounds!");

				int frame_count = data->keyframe_count / data->stride;
				for (int j = 0; j < frame_count; ++j) {
					int start_frame = j * data->stride;
					// printf("%d\n", start_frame);
					v3 curr = data->keyframes[start_frame + frame];
					v3 next = data->keyframes[start_frame + ((frame == data->stride-1) ? 0:frame+1)];

					instance.skeleton.position[j] = lerp(curr, next, t);
				}
			}

			// for (int i = 0; i < count; ++i) {
			// 	Instance &instance = instances[i];
			// 	v3 *vertices = SCRATCH_ALLOCATE(v3, instance.skeleton.count);
			// 	skeleton_get_vertices(instance.skeleton, vertices);
			// 	CALL(entity, model, update_vertices, vertices);
			// }
		}
	};
}
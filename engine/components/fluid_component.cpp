#include "engine/utils/math/random.h"

#define PARTICLE_COUNT 1024 * 2 // Must be a power of two

#include "fluid_common.cpp"

#define SPOOK 1

#if defined(MULLER)
#include "fluid_muller.cpp"
#elif defined(SPOOK)
#include "fluid_spook.cpp"
#elif defined(SPOOK_PTHREAD)
#include "fluid_spook_pthread.cpp"
#elif defined(MPM)
#include "fluid_mpm.cpp"
#endif

struct Buffer {
	GLuint vbo;
	cl_mem mem;
};
inline v2 zero(u64 i) {
	return V2_f32(0.0f, 0.0f);
}
inline v2 gen_random_pos(u64 i) {
	Random r;
	random_init(r, __rdtsc(), 54u);
	float x = random_f32(r);
	x *= 9;
	float y = random_f32(r);
	y *= 9;
	return V2_f32(x, y);
}

Buffer gen_buffer(/*cl_context context, */v2 (*f)(u64 i)) {
	Buffer buffer = {};
	glGenBuffers(1, &buffer.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
	v2 array[PARTICLE_COUNT];
	for (u64 i = 0; i < PARTICLE_COUNT; ++i) {
		array[i] = f(i);
	}
	glBufferData(GL_ARRAY_BUFFER, sizeof(v2)*PARTICLE_COUNT, array, GL_DYNAMIC_DRAW);

	// cl_int errcode_ret;
	// buffer.mem = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, buffer.vbo, &errcode_ret);
	// CL_CHECK_ERRORCODE(clCreateFromGLBuffer, errcode_ret);

	return buffer;
}

namespace fluid_component {
	struct Instance {
		Renderable renderable;

		GLenum buffer_type; // e.g. GL_STATIC_DRAW;
		Buffer positions;
		Buffer velocities;
		Buffer density_pressure;

		i32 vertex_count;
		i32 __padding;
	};

	struct FluidComponent {
		Instance instances[8];
		i32 count;

		cid add() {
			ASSERT((u32)count < ARRAY_COUNT(instances), "Component full!");
			i32 id = count++;
			Instance &instance = instances[id];
			Renderable &renderable = instance.renderable;

			renderable.pose = identity();

			renderable.index_count = PARTICLE_COUNT;
			renderable.datatype = RenderableDataType_Arrays;
			renderable.draw_mode = GL_POINTS;

			glGenVertexArrays(1, &renderable.vertex_array_object);
			glBindVertexArray(renderable.vertex_array_object);

			instance.positions = gen_buffer(gen_random_pos);
			instance.velocities = gen_buffer(zero);
			instance.density_pressure = gen_buffer(zero);

			glBindBuffer(GL_ARRAY_BUFFER, instance.positions.vbo);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, instance.density_pressure.vbo);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(1);

			return id;
		}

		inline void rotate(i32 id, float angle) {
			Instance &instance = instances[id];

			float ca = cosf(angle);
			float sa = sinf(angle);

			m4 rotation = identity();

			rotation.m[INDEX(0, 0)] = ca;
			rotation.m[INDEX(0, 1)] = -sa;
			rotation.m[INDEX(1, 0)] = sa;
			rotation.m[INDEX(1, 1)] = ca;

			instance.renderable.pose *= rotation;
		}

		void update(f32 dt) {
			for (i32 i = 0; i < count; ++i) {
				Instance &instance = instances[i];

				glBindBuffer(GL_ARRAY_BUFFER, instance.density_pressure.vbo);
				v2 *gpu_density_pressure = (v2*) glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);

				glBindBuffer(GL_ARRAY_BUFFER, instance.velocities.vbo);
				v2 *gpu_velocities = (v2*) glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);

				glBindBuffer(GL_ARRAY_BUFFER, instance.positions.vbo);
				v2 *gpu_positions = (v2*) glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);

				v2 density_pressure[PARTICLE_COUNT] = {};
				ALIGNED_(32) v2 velocities[PARTICLE_COUNT];
				ALIGNED_(32) v2 positions[PARTICLE_COUNT];

				// memcpy(density_pressure, gpu_density_pressure, PARTICLE_COUNT* sizeof(v2));
				memcpy(velocities, gpu_velocities, PARTICLE_COUNT* sizeof(v2));
				memcpy(positions, gpu_positions, PARTICLE_COUNT* sizeof(v2));

				float cos_angle = instance.renderable.pose.m[INDEX(0, 0)];
				float sin_angle = instance.renderable.pose.m[INDEX(1, 0)];

				v2 gravity = V2_f32(0, -9.82f);
				float gx = cos_angle * gravity.x - sin_angle * gravity.y;
				float gy = sin_angle * gravity.x + cos_angle * gravity.y;
				gravity.x = -gx;
				gravity.y = gy;

				fluid::simulate(positions, velocities, density_pressure, gravity);
				memset(density_pressure, 0, sizeof(density_pressure));
				fluid::simulate(positions, velocities, density_pressure, gravity);
				memset(density_pressure, 0, sizeof(density_pressure));
				fluid::simulate(positions, velocities, density_pressure, gravity);
				memset(density_pressure, 0, sizeof(density_pressure));
				fluid::simulate(positions, velocities, density_pressure, gravity);

				memcpy(gpu_density_pressure, density_pressure, PARTICLE_COUNT* sizeof(v2));
				memcpy(gpu_velocities, velocities, PARTICLE_COUNT* sizeof(v2));
				memcpy(gpu_positions, positions, PARTICLE_COUNT* sizeof(v2));

				glBindBuffer(GL_ARRAY_BUFFER, instance.density_pressure.vbo);
				glUnmapBuffer(GL_ARRAY_BUFFER);

				glBindBuffer(GL_ARRAY_BUFFER, instance.velocities.vbo);
				glUnmapBuffer(GL_ARRAY_BUFFER);

				glBindBuffer(GL_ARRAY_BUFFER, instance.positions.vbo);
				glUnmapBuffer(GL_ARRAY_BUFFER);
			}
		}
	};
}

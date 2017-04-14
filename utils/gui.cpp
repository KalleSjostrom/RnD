// #include "../utils/string_utils.inl"
#include "../utils/list_utils.inl"

#define MAX_STRING_LENGTH 256
#define MAX_JOBS 1024
#define MAX_DYNAMIC_JOBS 512
#define FIRST_INDEX_STATIC (MAX_JOBS-MAX_DYNAMIC_JOBS)

#define INV_WIDTH (1.0f/RES_WIDTH)
#define INV_HEIGHT (1.0f/RES_HEIGHT)

namespace gui {

	static v4 WHITE = { 1.0f, 1.0f, 1.0f, 1.0f };

	struct Job {
		u16 string_length;
		float width;
		GLuint vbo[2];
		GLuint vao[1];

	    char DEBUG_NAME[32];
	};

	struct GUI {
		font::Font font;
		MemoryArena *transient_arena;

		GLuint texture;
		GLint program;
		GLuint index_vertex_buffer;

		GLuint location_projection;
		GLuint location_color;

		m4 projection;

		u16 static_job_count;
		Job jobs[MAX_JOBS];
		ListHeader list_header;
		Link dynamic_job_list[MAX_DYNAMIC_JOBS];
	};

	GUI *init(MemoryArena &persistent_arena, MemoryArena &transient_arena) {
		GUI *gui = (GUI *)allocate_memory(persistent_arena, sizeof(GUI));

		font::load(persistent_arena, "assets/font.gamefont", &gui->font);
		gui->transient_arena = &transient_arena;
		int width = gui->font.width;
		int height = gui->font.height;
		glGenTextures(1, &gui->texture);
		glBindTexture(GL_TEXTURE_2D, gui->texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, gui->font.pixels);

		gui->program = gl_program_builder::create_from_source_file("./shaders/text.vert", "./shaders/text.frag");
		glUseProgram(gui->program);

		gui->location_color = glGetUniformLocation(gui->program, "color");
		glUniform4f(gui->location_color, 1.0f, 1.0f, 1.0f, 1.0f);

		gui->location_projection = glGetUniformLocation(gui->program, "projection");
		gui->projection = orthographic(RES_WIDTH, 0, 0, RES_HEIGHT, 0, 1);
		glUniformMatrix4fv(gui->location_projection, 1, GL_FALSE, (GLfloat*)(gui->projection.m));

		static u32 index_buffer_size = (6 * MAX_STRING_LENGTH * sizeof(GLushort));
		GLushort index_buffer[index_buffer_size];

		for (int i = 0; i < MAX_STRING_LENGTH; ++i) {
			index_buffer[i*6+0] = i*4+0;
			index_buffer[i*6+1] = i*4+1;
			index_buffer[i*6+2] = i*4+2;
			index_buffer[i*6+3] = i*4+2;
			index_buffer[i*6+4] = i*4+1;
			index_buffer[i*6+5] = i*4+3;
		}

		glGenBuffers(1, &gui->index_vertex_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gui->index_vertex_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size, index_buffer, GL_STATIC_DRAW);

		for (int i = 0; i < MAX_JOBS; i++) {
			Job &job = gui->jobs[i];
			glGenBuffers(2, job.vbo);
			glGenVertexArrays(1, job.vao);

			glBindVertexArray(job.vao[0]);

			glBindBuffer(GL_ARRAY_BUFFER, job.vbo[0]);
			int position_location = glGetAttribLocation(gui->program, "position");
			glVertexAttribPointer(position_location, 2, GL_FLOAT, false, 0, 0);
			glEnableVertexAttribArray(0);

			glBindBuffer(GL_ARRAY_BUFFER, job.vbo[1]);
			int uv_location = glGetAttribLocation(gui->program, "vertex_uv");
			glVertexAttribPointer(uv_location, 2, GL_FLOAT, false, 0, 0);
			glEnableVertexAttribArray(1);
		}

		initiate_list(gui->dynamic_job_list, MAX_DYNAMIC_JOBS, gui->list_header);

		return gui;
	}

	static void _fill_buffer_data(GUI *gui, String string, float x, float y, float *position, float *uv) {
		int string_length = string.length;
		font::Font &font = gui->font;

		for (int i = 0; i < string_length; i++) {
			font::FontCharacter &c = font.characters[string[i] - ' '];

			// Front facing is counter clockwise, but we want to scale with negative y in order to use the top left scheme that's already in place
			float basex = x+c.xoff;
			float basey = y+c.yoff;

			position[i*8+0] = basex;
			position[i*8+1] = basey;
			position[i*8+2] = basex;
			position[i*8+3] = basey + c.height;
			position[i*8+4] = basex + c.width;
			position[i*8+5] = basey;
			position[i*8+6] = basex + c.width;
			position[i*8+7] = basey + c.height;

			uv[i*8+0] = c.left;
			uv[i*8+1] = c.bottom;
			uv[i*8+2] = c.left;
			uv[i*8+3] = c.top;
			uv[i*8+4] = c.right;
			uv[i*8+5] = c.bottom;
			uv[i*8+6] = c.right;
			uv[i*8+7] = c.top;

			x += c.xadvance;
			if (i < string_length-1) {
				float kern = font::get_kerning(font.kerning_table, string[i], string[i+1]);
				x += kern;
			}
		}
	}

	void update_string(GUI *gui, u16 handle, String &string, float x = 0.0f, float y = 0.0f) {
		// Allocate the position and uv buffers, fill them and then send them to the gpu.
		MemoryBlockHandle memory_block_handle = begin_block(*gui->transient_arena);
			Job &job = gui->jobs[handle];

			int string_length = string.length;
			int buffer_length = 8 * string_length;
			int buffer_size = buffer_length * sizeof(float);

			float *quad_buffer = (float *) allocate_memory(*gui->transient_arena, buffer_size);
			float *quad_buffer_uv = (float *) allocate_memory(*gui->transient_arena, buffer_size);

			_fill_buffer_data(gui, string, x, y, quad_buffer, quad_buffer_uv);

			glBindBuffer(GL_ARRAY_BUFFER, job.vbo[0]);
			glBufferData(GL_ARRAY_BUFFER, buffer_size, quad_buffer, GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, job.vbo[1]);
			glBufferData(GL_ARRAY_BUFFER, buffer_size, quad_buffer_uv, GL_STATIC_DRAW);

			job.string_length = string_length;
			job.width = quad_buffer[buffer_length-2] - quad_buffer[0];
		end_block(*gui->transient_arena, memory_block_handle);
	}

	int create_string(GUI *gui, bool dynamic, String &string, float x = 0.0f, float y = 0.0f) {
		// Get a new free job.
		u16 handle;
		if (dynamic) {
			activate_first_free(gui->dynamic_job_list, gui->list_header);
			handle = gui->list_header.first_active;
		} else {
			handle = gui->static_job_count;
			gui->static_job_count = (handle + 1) % (MAX_JOBS - FIRST_INDEX_STATIC);
			handle += FIRST_INDEX_STATIC;
		}

		update_string(gui, handle, string, x, y);

#if STRING_DEBUG
		Job &job = gui->jobs[handle];
		int size = string_length > 31 ? 31 : string_length;
	    for (int i = 0; i < size; i++) {
	    	job.DEBUG_NAME[i] = string[i];
	    }
	    job.DEBUG_NAME[size] = '\0';
#endif // STRING_DEBUG
		return handle;
	}

	void destroy_string(GUI *gui, int handle) {
		bool is_static = handle >= FIRST_INDEX_STATIC;
		Job &job = gui->jobs[handle];
		if (is_static) {
			gui->jobs[handle] = gui->jobs[FIRST_INDEX_STATIC + gui->static_job_count--];
		} else {
			free_active(gui->dynamic_job_list, handle, gui->list_header);
		}
	}

	void render(GUI *gui, int handle_from, int nr_jobs, Job *jobs, float x, float y, float scale, v4 color) {
		gui->projection.m[12] = -(RES_WIDTH-2.0f*x) * INV_WIDTH;
		gui->projection.m[13] = (RES_HEIGHT-2.0f*y) * INV_HEIGHT;

		gui->projection.m[0] = (2*scale) * INV_WIDTH;
		gui->projection.m[5] = -(2*scale) * INV_HEIGHT;

		glUniformMatrix4fv(gui->location_projection, 1, GL_FALSE, (GLfloat*)(gui->projection.m));
		glUniform4f(gui->location_color, color.r, color.g, color.b, color.a);

		for (int i = handle_from; i < handle_from+nr_jobs; i++) {
			Job &job = jobs[i];
			glBindVertexArray(job.vao[0]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gui->index_vertex_buffer);
			glDrawElements(GL_TRIANGLES, 6 * job.string_length, GL_UNSIGNED_SHORT, (void*)0);
		}
	}

	inline void begin_render(GUI *gui) {
		glUseProgram(gui->program);
		glBindTexture(GL_TEXTURE_2D, gui->texture);
	}

	void render_dynamic(GUI *gui, int handle, float x = 0.0f, float y = 0.0f, float scale = 1.0f, v4 color = WHITE) {
		render(gui, handle, 1, gui->jobs, x, y, scale, color);
	}

	void render_all_static(GUI *gui, float x = 0.0f, float y = 0.0f, float scale = 1.0f, v4 color = WHITE) {
		begin_render(gui);
		render(gui, FIRST_INDEX_STATIC, MAX_JOBS-MAX_DYNAMIC_JOBS, gui->jobs, x, y, scale, color);
	}
}

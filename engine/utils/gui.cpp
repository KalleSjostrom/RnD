#include "string.h"
#include "containers/linked_list.h"
#include "font_loader.cpp"

#define STRING_DEBUG 0

#define MAX_STRING_LENGTH 256
#define MAX_JOBS 1024
#define MAX_DYNAMIC_JOBS 512
#define FIRST_INDEX_STATIC (MAX_JOBS-MAX_DYNAMIC_JOBS)

#define INV_WIDTH (1.0f/RES_WIDTH)
#define INV_HEIGHT (1.0f/RES_HEIGHT)

static v4 WHITE = { 1.0f, 1.0f, 1.0f, 1.0f };

struct StringJob {
	i32 string_length;
	f32 width;
	GLuint vbo[2];
	GLuint vao[1];

    char DEBUG_NAME[32];
};

struct GUI {
	m4 projection;

	font::Font font;
	MemoryArena *transient_arena;

	GLuint texture;
	GLuint program;
	GLuint index_vertex_buffer;

	GLint location_projection;
	GLint location_color;

	i32 static_job_count;
	StringJob jobs[MAX_JOBS];
	ListHeader list_header;
	Link dynamic_job_list[MAX_DYNAMIC_JOBS];
};

struct GUISettings {
	const char *font_path;
	const char *text_vertex_shader;
	const char *text_fragment_shader;
};

void init(GUI &gui, MemoryArena &persistent_arena, MemoryArena &transient_arena, GUISettings &settings) {
	font::load(persistent_arena, settings.font_path, &gui.font);
	gui.transient_arena = &transient_arena;
	i32 width = gui.font.width;
	i32 height = gui.font.height;
	glGenTextures(1, &gui.texture);
	glBindTexture(GL_TEXTURE_2D, gui.texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, gui.font.pixels);

	gui.program = gl_program_builder::create_from_source_files(transient_arena, settings.text_vertex_shader, settings.text_fragment_shader, 0);
	glUseProgram(gui.program);

	gui.location_color = glGetUniformLocation(gui.program, "color");
	glUniform4f(gui.location_color, 1.0f, 1.0f, 1.0f, 1.0f);

	gui.location_projection = glGetUniformLocation(gui.program, "projection");
	gui.projection = orthographic(RES_WIDTH, 0, 0, RES_HEIGHT, 0, 1);
	glUniformMatrix4fv(gui.location_projection, 1, GL_FALSE, (GLfloat*)(gui.projection.m));

	#define INDEX_BUFFER_SIZE (6 * MAX_STRING_LENGTH * sizeof(GLushort))
	GLushort index_buffer[INDEX_BUFFER_SIZE];

	for (i32 i = 0; i < MAX_STRING_LENGTH; ++i) {
		index_buffer[i*6+0] = (GLushort)i*4+0;
		index_buffer[i*6+1] = (GLushort)i*4+1;
		index_buffer[i*6+2] = (GLushort)i*4+2;
		index_buffer[i*6+3] = (GLushort)i*4+2;
		index_buffer[i*6+4] = (GLushort)i*4+1;
		index_buffer[i*6+5] = (GLushort)i*4+3;
	}

	glGenBuffers(1, &gui.index_vertex_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gui.index_vertex_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDEX_BUFFER_SIZE, index_buffer, GL_STATIC_DRAW);

	for (i32 i = 0; i < MAX_JOBS; i++) {
		StringJob &job = gui.jobs[i];
		glGenBuffers(2, job.vbo);
		glGenVertexArrays(1, job.vao);

		glBindVertexArray(job.vao[0]);

		glBindBuffer(GL_ARRAY_BUFFER, job.vbo[0]);
		GLint position_location = glGetAttribLocation(gui.program, "position");
		ASSERT(position_location != -1, "Invalid attribute location for gui program!");
		glVertexAttribPointer((GLuint) position_location, 2, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, job.vbo[1]);
		GLint uv_location = glGetAttribLocation(gui.program, "vertex_uv");
		ASSERT(uv_location != -1, "Invalid attribute location for gui program!");
		glVertexAttribPointer((GLuint) uv_location, 2, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(1);
	}

	initiate_list(gui.dynamic_job_list, MAX_DYNAMIC_JOBS, gui.list_header);
}

static void _fill_buffer_data(GUI &gui, String string, f32 x, f32 y, f32 *position, f32 *uv) {
	i32 string_length = string.length;
	font::Font &font = gui.font;

	for (i32 i = 0; i < string_length; i++) {
		font::FontCharacter &c = font.characters[string[i] - ' '];

		// Front facing is counter clockwise, but we want to scale with negative y in order to use the top left scheme that's already in place
		f32 basex = x+c.xoff;
		f32 basey = y+c.yoff;

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
			f32 kern = font::get_kerning(font.kerning_table, string[i], string[i+1]);
			x += kern;
		}
	}
}

void update_string(GUI &gui, i32 handle, String &string, f32 x = 0.0f, f32 y = 0.0f) {
	// Allocate the position and uv buffers, fill them and then send them to the gpu.
	MemoryBlockHandle memory_block_handle = begin_block(*gui.transient_arena);
		StringJob &job = gui.jobs[handle];

		u32 string_length = (u32) string.length;
		u32 buffer_length = 8 * string_length;
		size_t buffer_size = buffer_length * sizeof(f32);

		f32 *quad_buffer = PUSH_STRUCTS(*gui.transient_arena, buffer_length, f32);
		f32 *quad_buffer_uv = PUSH_STRUCTS(*gui.transient_arena, buffer_length, f32);

		_fill_buffer_data(gui, string, x, y, quad_buffer, quad_buffer_uv);

		glBindBuffer(GL_ARRAY_BUFFER, job.vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)buffer_size, quad_buffer, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, job.vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)buffer_size, quad_buffer_uv, GL_STATIC_DRAW);

		job.string_length = (i32)string_length;
		job.width = quad_buffer[buffer_length-2] - quad_buffer[0];
	end_block(*gui.transient_arena, memory_block_handle);
}

i32 create_string(GUI &gui, bool dynamic, String &string, f32 x = 0.0f, f32 y = 0.0f) {
	// Get a new free job.
	i16 handle;
	if (dynamic) {
		activate_first_free(gui.dynamic_job_list, gui.list_header);
		handle = gui.list_header.first_active;
	} else {
		handle = (i16)gui.static_job_count;
		gui.static_job_count = (handle + 1) % (MAX_JOBS - FIRST_INDEX_STATIC);
		handle += FIRST_INDEX_STATIC;
	}

	update_string(gui, handle, string, x, y);

#if STRING_DEBUG
	StringJob &job = gui.jobs[handle];
	i32 size = string.length > 31 ? 31 : string.length;
    for (i32 i = 0; i < size; i++) {
    	job.DEBUG_NAME[i] = string[i];
    }
    job.DEBUG_NAME[size] = '\0';
#endif // STRING_DEBUG
	return handle;
}

void destroy_string(GUI &gui, i32 handle) {
	bool is_static = handle >= FIRST_INDEX_STATIC;
	if (is_static) {
		gui.jobs[handle] = gui.jobs[FIRST_INDEX_STATIC + gui.static_job_count--];
	} else {
		free_active(gui.dynamic_job_list, (i16)handle, gui.list_header);
	}
}

void render(GUI &gui, i32 handle_from, i32 nr_jobs, StringJob *jobs, f32 x, f32 y, f32 scale, v4 color) {
	gui.projection.m[12] = -(RES_WIDTH-2.0f*x) * INV_WIDTH;
	gui.projection.m[13] = (RES_HEIGHT-2.0f*y) * INV_HEIGHT;

	gui.projection.m[0] = (2*scale) * INV_WIDTH;
	gui.projection.m[5] = -(2*scale) * INV_HEIGHT;

	glUniformMatrix4fv(gui.location_projection, 1, GL_FALSE, (GLfloat*)(gui.projection.m));
	glUniform4f(gui.location_color, color.r, color.g, color.b, color.a);

	for (i32 i = handle_from; i < handle_from+nr_jobs; i++) {
		StringJob &job = jobs[i];
		glBindVertexArray(job.vao[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gui.index_vertex_buffer);
		glDrawElements(GL_TRIANGLES, 6 * (i32)job.string_length, GL_UNSIGNED_SHORT, (void*)0);
	}
}

inline void begin_render(GUI &gui) {
	glUseProgram(gui.program);
	glBindTexture(GL_TEXTURE_2D, gui.texture);
}

void render_dynamic(GUI &gui, i32 handle, f32 x = 0.0f, f32 y = 0.0f, f32 scale = 1.0f, v4 color = WHITE) {
	render(gui, handle, 1, gui.jobs, x, y, scale, color);
}

void render_all_static(GUI &gui, f32 x = 0.0f, f32 y = 0.0f, f32 scale = 1.0f, v4 color = WHITE) {
	begin_render(gui);
	render(gui, FIRST_INDEX_STATIC, MAX_JOBS-MAX_DYNAMIC_JOBS, gui.jobs, x, y, scale, color);
}

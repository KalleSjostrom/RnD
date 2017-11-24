#include "engine/utils/string.h"

struct GroupData {
	i32 index_count;
	GLindex *indices;

	i32 material_index;
};

enum IlluminationMode {
	IlluminationMode_ColorOn_AmbientOff = 0, // 0. Color on and Ambient off
	IlluminationMode_ColorOn_AmbientOn = 1, // 1. Color on and Ambient on
	IlluminationMode_HighlightOn = 2, // 2. Highlight on
	IlluminationMode_ReflectionOn_RayTraceOn = 3, // 3. Reflection on and Ray trace on
	IlluminationMode_Transparency = 4, // 4. Transparency: Glass on, Reflection: Ray trace on
	// 5. Reflection: Fresnel on and Ray trace on
	// 6. Transparency: Refraction on, Reflection: Fresnel off and Ray trace on
	// 7. Transparency: Refraction on, Reflection: Fresnel on and Ray trace on
	// 8. Reflection on and Ray trace off
	// 9. Transparency: Glass on, Reflection: Ray trace off
	// 10. Casts shadows onto invisible surfaces
};

struct MaterialData {
	float Ns; // Specular exponent
	float Ni; // optical_density - Index of refraction
	float Tr; // Translucent (1 - d)
	float Tf; // Transmission filter
	IlluminationMode illum; // Illumination mode
	v3 Ka; // Ambient color
	v3 Kd; // Diffuse color
	v3 Ks; // Specular color
	v3 Ke; // Emissive color
	String map_Ka; // Path to ambient texture
	String map_Kd; // Path to diffuse texture
	String map_Ks; // Path to specular texture
	String map_Ke; // Path to emissive texture
	String map_d; // Path to translucency mask texture
	String map_bump; // Path to bump map texture
};

struct Material {
	float Ns; // Specular exponent
	float Ni; // optical_density - Index of refraction
	float Tr; // Translucent (1 - d)
	float Tf; // Transmission filter
	IlluminationMode illum; // Illumination mode
	v3 Ka; // Ambient color
	v3 Kd; // Diffuse color
	v3 Ks; // Specular color
	v3 Ke; // Emissive color
	GLuint map_Ka; // ambient texture
	GLuint map_Kd; // diffuse texture
	GLuint map_Ks; // specular texture
	GLuint map_Ke; // emissive texture
	GLuint map_d; // translucency mask texture
	GLuint map_bump; // bump map texture
};

struct MeshData {
	i32 vertex_count;
	i32 coord_count;
	i32 normal_count;
	i32 group_count;
	i32 material_count;

	v3 *vertices;
	v2 *coords;
	v3 *normals;
	GroupData *groups;
	MaterialData *materials;
};

enum RenderableDataType {
	RenderableDataType_Elements,
	RenderableDataType_Arrays,
};

struct Channel {
	GLuint vertex_buffer_object;
};

struct Group {
	i32 index_count;
	GLuint element_array_buffer;
	Material *material;
};

struct Mesh {
	i32 group_count;
	i32 material_count;

	Group *groups;
	Material *materials;

	// Channels - vertex array
	GLuint vertex_array_object;
	Channel vertex;
	Channel normal;
	Channel tex_coord;
};

struct Renderable {
	m4 pose;
	RenderableDataType datatype;
	GLenum draw_mode; // e.g. GL_TRIANGLE_STRIP;

	Mesh mesh;
};

struct Model {
	Renderable renderable;

	GLenum buffer_type; // e.g. GL_STATIC_DRAW;

	i32 vertex_count;
	i32 __padding;
};

struct ModelCC {
	MeshData mesh_data;

	GLenum buffer_type; //  = GL_STATIC_DRAW
	GLenum draw_mode; // = GL_TRIANGLE_STRIP

	i32 program_type;
};

GLuint load_white() {
	GLuint texture;
	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	unsigned pixel = 0xffffffff;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, &pixel);
	return texture;
}

GLuint load_texture(EngineApi *engine, String &path, GLuint default_texture = 0) {
	if (path.length == 0) {
		return default_texture;
	}

	ImageData image_data;
	char buf[1024] = {};
	int count = snprintf(buf, ARRAY_COUNT(buf), "../../conetracer/assets/%.*s", path.length, *path);
	for (int i = 0; i < count; ++i) {
		if (buf[i] == '\\')
			buf[i] = '/';
	}

	b32 success = engine->image_load(buf, image_data);
	if (!success) {
		LOG_ERROR("Model", "Could not load image '%s'!\n", buf);
		return default_texture;
	}

	GLuint texture;
	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// The png is stored as ARGB
	// The tga is stored as BGR
	switch(image_data.format) {
		case PixelFormat_RGBA: {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_data.width, image_data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data.pixels);
		} break;
		case PixelFormat_ARGB: {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_data.width, image_data.height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image_data.pixels);
		} break;
		case PixelFormat_RGB: {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_data.width, image_data.height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data.pixels);
		} break;
		case PixelFormat_BGR: {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_data.width, image_data.height, 0, GL_BGR, GL_UNSIGNED_BYTE, image_data.pixels);
		} break;
	}


	glGenerateMipmap(GL_TEXTURE_2D); // This has to happen after we've sent the pixel-data to the card, i.e. after glTexImage2D

	return texture;
}

struct ModelComponent {
	Model instances[32];
	cid count;

	cid add(EngineApi *engine, MemoryArena &arena, v3 position, ModelCC *cc) {
		ASSERT((u32)count < ARRAY_COUNT(instances), "Component full!");
		cid id = count++;
		Model &instance = instances[id];
		Renderable &renderable = instance.renderable;

		renderable.pose = identity();
		set_position(id, position);

		instance.buffer_type = cc->buffer_type;
		renderable.draw_mode = cc->draw_mode;
		renderable.datatype = RenderableDataType_Elements;

		MeshData &mesh_data = cc->mesh_data;
		Mesh &mesh = renderable.mesh;

		mesh.material_count = mesh_data.material_count;
		mesh.materials = PUSH_STRUCTS(arena, mesh.material_count, Material);

		GLuint white = load_white();

		for (i32 material_index = 0; material_index < mesh_data.material_count; ++material_index) {
			MaterialData &material_data = mesh_data.materials[material_index];
			Material &material = mesh.materials[material_index];

			material.Ns    = material_data.Ns;
			material.Ni    = material_data.Ni;
			material.Tr    = material_data.Tr;
			material.Tf    = material_data.Tf;
			material.illum = material_data.illum;
			material.Ka    = material_data.Ka;
			material.Kd    = material_data.Kd;
			material.Ks    = material_data.Ks;
			material.Ke    = material_data.Ke;

			material.map_Ka   = load_texture(engine, material_data.map_Ka);
			material.map_Kd   = load_texture(engine, material_data.map_Kd);
			material.map_Ks   = load_texture(engine, material_data.map_Ks);
			material.map_Ke   = load_texture(engine, material_data.map_Ke);
			material.map_d    = load_texture(engine, material_data.map_d, white);
			material.map_bump = load_texture(engine, material_data.map_bump);
		}

		mesh.group_count = mesh_data.group_count;
		mesh.groups = PUSH_STRUCTS(arena, mesh.group_count, Group);

		for (i32 group_index = 0; group_index < mesh.group_count; ++group_index) {
			GroupData &group_data = mesh_data.groups[group_index];
			Group &group = mesh.groups[group_index];

			if (group_data.material_index >= 0) {
				group.material = &mesh.materials[group_data.material_index];
			} else {
				group.material = 0;
			}

			group.index_count = group_data.index_count;

			glGenBuffers(1, &group.element_array_buffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, group.element_array_buffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (u32)group.index_count * sizeof(GLindex), group_data.indices, GL_STATIC_DRAW); // Indices are not assumed to change, only the vertices
		}

		glGenVertexArrays(1, &mesh.vertex_array_object);

		{ // Vertex channel
			Channel &c = mesh.vertex;
			if (mesh_data.vertex_count) {
				glGenBuffers(1, &c.vertex_buffer_object);
				glBindBuffer(GL_ARRAY_BUFFER, c.vertex_buffer_object);
				glBufferData(GL_ARRAY_BUFFER, (u32)mesh_data.vertex_count * sizeof(v3), mesh_data.vertices, cc->buffer_type);

				glBindVertexArray(mesh.vertex_array_object);

				glBindBuffer(GL_ARRAY_BUFFER, c.vertex_buffer_object);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(0);
			}
		}

		{ // Normal channel
			Channel &c = mesh.normal;
			if (mesh_data.normal_count) {
				glGenBuffers(1, &c.vertex_buffer_object);
				glBindBuffer(GL_ARRAY_BUFFER, c.vertex_buffer_object);
				glBufferData(GL_ARRAY_BUFFER, (u32)mesh_data.normal_count * sizeof(v3), mesh_data.normals, cc->buffer_type);

				glBindVertexArray(mesh.vertex_array_object);

				glBindBuffer(GL_ARRAY_BUFFER, c.vertex_buffer_object);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(1);
			}
		}

		{ // Tex coord channel
			Channel &c = mesh.tex_coord;
			if (mesh_data.coord_count) {
				glGenBuffers(1, &c.vertex_buffer_object);
				glBindBuffer(GL_ARRAY_BUFFER, c.vertex_buffer_object);
				glBufferData(GL_ARRAY_BUFFER, (u32)mesh_data.coord_count * sizeof(v2), mesh_data.coords, cc->buffer_type);

				glBindVertexArray(mesh.vertex_array_object);

				glBindBuffer(GL_ARRAY_BUFFER, c.vertex_buffer_object);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(2);
			}
		}

		return id;
	}

	inline void set_position(i32 id, v3 position) {
		Model &instance = instances[id];
		translation(instance.renderable.pose) = position;
	}

	inline v3 get_position(i32 id) {
		Model &instance = instances[id];
		return translation(instance.renderable.pose);
	}

	inline m4 &get_pose(i32 id) {
		Model &instance = instances[id];
		return instance.renderable.pose;
	}

	inline void rotate_around(i32 id, q4 q) {
		Model &instance = instances[id];

		m4 &pose = instance.renderable.pose;

		v3 &x = *(v3*)(pose.m + 0);
		v3 &y = *(v3*)(pose.m + 4);
		v3 &z = *(v3*)(pose.m + 8);

		x = ::rotate_around(q, x);
		y = ::rotate_around(q, y);
		z = ::rotate_around(q, z);
	}

	inline void rotate_around(i32 id, float angle, float x, float y) {
		Model &instance = instances[id];

		float ca = cosf(angle);
		float sa = sinf(angle);

		float rx = -ca*x + sa*y + x;
		float ry = -sa*x - ca*y + y;

		m4 rotation = identity();

		rotation.m[INDEX(0, 3)] = rx;
		rotation.m[INDEX(1, 3)] = ry;

		rotation.m[INDEX(0, 0)] = ca;
		rotation.m[INDEX(0, 1)] = -sa;
		rotation.m[INDEX(1, 0)] = sa;
		rotation.m[INDEX(1, 1)] = ca;

		instance.renderable.pose *= rotation;
	}

	inline void rotate(i32 id, float angle) {
		Model &instance = instances[id];

		float ca = cosf(angle);
		float sa = sinf(angle);

		m4 rotation = identity();

		rotation.m[INDEX(0, 0)] = ca;
		rotation.m[INDEX(0, 1)] = -sa;
		rotation.m[INDEX(1, 0)] = sa;
		rotation.m[INDEX(1, 1)] = ca;

		instance.renderable.pose *= rotation;
	}

	inline void set_rotation(i32 id, float angle) {
		Model &instance = instances[id];

		float ca = cosf(angle);
		float sa = sinf(angle);

		m4 &pose = instance.renderable.pose;

		pose.m[INDEX(0, 0)] = ca;
		pose.m[INDEX(0, 1)] = -sa;
		pose.m[INDEX(1, 0)] = sa;
		pose.m[INDEX(1, 1)] = ca;

		// instance.renderable.pose = rotation;
	}

	inline void set_scale(i32 id, v3 scale) {
		Model &instance = instances[id];

		m4 &pose = instance.renderable.pose;

		pose.m[INDEX(0, 0)] = scale.x;
		pose.m[INDEX(1, 1)] = scale.y;
		pose.m[INDEX(2, 2)] = scale.z;
	}

	void update_vertices(i32 id, v3 *vertices) {
		// Model &instance = instances[id];

		// ASSERT(instance.buffer_type == GL_DYNAMIC_DRAW, "Should not update vertices with static buffer type!");
		// glBindBuffer(GL_ARRAY_BUFFER, instance.vertex_buffer_object);
		// glBufferData(GL_ARRAY_BUFFER, (u32)instance.vertex_count * sizeof(v3), vertices, GL_DYNAMIC_DRAW);
	}

	void update_vertices(i32 id, GLindex *indices, i32 index_count, v3 *vertices, i32 vertex_count) {
		// Model &instance = instances[id];

		// ASSERT(instance.buffer_type == GL_DYNAMIC_DRAW, "Should not update vertices with static buffer type!");

		// instance.renderable.index_count = index_count;
		// instance.vertex_count = vertex_count;
		// glBindBuffer(GL_ARRAY_BUFFER, instance.vertex_buffer_object);
		// glBufferData(GL_ARRAY_BUFFER, (u32)vertex_count * sizeof(v3), vertices, instance.buffer_type);

		// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance.renderable.element_array_buffer);
		// glBufferData(GL_ELEMENT_ARRAY_BUFFER, (u32)index_count * sizeof(GLindex), indices, GL_STATIC_DRAW); // Indices are not assumed to change, only the vertices
	}

	v3 transform_vertex(i32 id, v3 &vertex) {
		Model &instance = instances[id];

		return multiply_perspective(instance.renderable.pose, vertex);
	}

	void render(i32 id, GLint model_location) {
		Renderable &re = instances[id].renderable;

		glUniformMatrix4fv(model_location, 1, GL_FALSE, (GLfloat*)(re.pose.m));

		glBindVertexArray(re.mesh.vertex_array_object);

		for (i32 i = 0; i < re.mesh.group_count; ++i) {
			Group &group = re.mesh.groups[i];
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, group.element_array_buffer);
			glDrawElements(re.draw_mode, group.index_count, GL_UNSIGNED_INT, (void*)0);
		}
	}
};

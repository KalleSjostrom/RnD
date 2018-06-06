#include "engine/utils/string.h"
#include "engine/utils/texture.h"

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

struct GroupData {
	i32 index_count;
	GLindex *indices;

	i32 material_index;
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

enum RenderableDataType {
	RenderableDataType_Elements,
	RenderableDataType_Arrays,
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

	GLuint vertex_array_object;
	GLuint vertex_vbo;
	GLuint normal_vbo;
	GLuint tex_coord_vbo;
};

struct Renderable {
	m4 pose;
	RenderableDataType datatype;
	GLenum draw_mode; // e.g. GL_TRIANGLE_STRIP;
	GLenum buffer_type; // e.g. GL_STATIC_DRAW;

	Mesh mesh;
};

struct ModelCC {
	MeshData mesh_data;

	GLenum buffer_type; //  = GL_STATIC_DRAW
	GLenum draw_mode; // = GL_TRIANGLE_STRIP

	i32 program_type;
};

static String directory = MAKE_STRING(ASSET_FOLDER);

struct ModelComponent {
	Renderable models[32];
	cid count;
};

Renderable *get_model(ModelComponent &mc, Entity &entity) {
	return mc.models + entity.model_id;
}

inline m4 &get_pose(ModelComponent &mc, Entity &entity) {
	Renderable &model = mc.models[entity.model_id];
	return model.pose;
}

void add(ModelComponent &mc, Entity &entity, EngineApi *engine, MemoryArena &arena, v3 position, ModelCC *cc) {
	ASSERT((u32)mc.count < ARRAY_COUNT(mc.models), "Component full!");
	entity.model_id = mc.count++;
	Renderable &model = mc.models[entity.model_id];

	model.pose = identity();
	translation(model.pose) = position;

	model.buffer_type = cc->buffer_type;
	model.draw_mode = cc->draw_mode;
	model.datatype = RenderableDataType_Elements;

	MeshData &mesh_data = cc->mesh_data;
	Mesh &mesh = model.mesh;

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

		material.map_Ka   = load_texture(engine, directory, material_data.map_Ka);
		material.map_Kd   = load_texture(engine, directory, material_data.map_Kd, false);
		material.map_Ks   = load_texture(engine, directory, material_data.map_Ks);
		material.map_Ke   = load_texture(engine, directory, material_data.map_Ke);
		material.map_d    = load_texture(engine, directory, material_data.map_d, white);
		material.map_bump = load_texture(engine, directory, material_data.map_bump);
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

	GLuint channel = 0;
	{ // Vertex channel
		if (mesh_data.vertex_count) {
			glGenBuffers(1, &mesh.vertex_vbo);
			glBindBuffer(GL_ARRAY_BUFFER, mesh.vertex_vbo);
			glBufferData(GL_ARRAY_BUFFER, (u32)mesh_data.vertex_count * sizeof(v3), mesh_data.vertices, cc->buffer_type);

			glBindVertexArray(mesh.vertex_array_object);
			glVertexAttribPointer(channel, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(channel);
		}
	}
			channel++;

	{ // Normal channel
		if (mesh_data.normal_count) {
			glGenBuffers(1, &mesh.normal_vbo);
			glBindBuffer(GL_ARRAY_BUFFER, mesh.normal_vbo);
			glBufferData(GL_ARRAY_BUFFER, (u32)mesh_data.normal_count * sizeof(v3), mesh_data.normals, cc->buffer_type);

			glBindVertexArray(mesh.vertex_array_object);
			glVertexAttribPointer(channel, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(channel);
		}
	}
			channel++;

	{ // Tex coord channel
		if (mesh_data.coord_count) {
			glGenBuffers(1, &mesh.tex_coord_vbo);
			glBindBuffer(GL_ARRAY_BUFFER, mesh.tex_coord_vbo);
			glBufferData(GL_ARRAY_BUFFER, (u32)mesh_data.coord_count * sizeof(v2), mesh_data.coords, cc->buffer_type);

			glBindVertexArray(mesh.vertex_array_object);
			glVertexAttribPointer(channel, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(channel);
		}
	}
			channel++;
}

void render(ModelComponent &mc, Entity &entity, GLint model_location) {
	Renderable &re = mc.models[entity.model_id];

	glUniformMatrix4fv(model_location, 1, GL_FALSE, (GLfloat*)(re.pose.m));

	glBindVertexArray(re.mesh.vertex_array_object);

	for (i32 i = 0; i < re.mesh.group_count; ++i) {
		Group &group = re.mesh.groups[i];
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, group.element_array_buffer);
		glDrawElements(re.draw_mode, group.index_count, GL_UNSIGNED_INT, (void*)0);
	}
}

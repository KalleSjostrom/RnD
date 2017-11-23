#include "engine/utils/string.h"

void read_string(MemoryArena &arena, String string, FILE *file) {
	fread(&string.length, sizeof(i32), 1, file);
	string.text = PUSH_STRING(arena, string.length);
	fread(&string.text, sizeof(char), string.length, file);
}

MeshData read_obj(MemoryArena &arena, const char *filepath) {
	FILE *objfile;
	fopen_s(&objfile, filepath, "rb");
	ASSERT(objfile, "Could not open '%s'", filepath);

	MeshData mesh = {};

	fread(&mesh.vertex_count, sizeof(int), 1, objfile);
	fread(&mesh.coord_count, sizeof(int), 1, objfile);
	fread(&mesh.normal_count, sizeof(int), 1, objfile);
	fread(&mesh.group_count, sizeof(int), 1, objfile);

	mesh.vertices = PUSH_STRUCTS(arena, mesh.vertex_count, v3);
	mesh.coords = PUSH_STRUCTS(arena, mesh.coord_count, v2);
	mesh.normals = PUSH_STRUCTS(arena, mesh.normal_count, v3);
	mesh.groups = PUSH_STRUCTS(arena, mesh.group_count, GroupData);

	fread(mesh.vertices, sizeof(v3), mesh.vertex_count, objfile);
	fread(mesh.coords, sizeof(v2), mesh.coord_count, objfile);
	fread(mesh.normals, sizeof(v3), mesh.normal_count, objfile);

	for (int i = 0; i < mesh.group_count; ++i) {
		GroupData &group = mesh.groups[i];
		fread(&group.index_count, sizeof(int), 1, objfile);

		group.indices = PUSH_STRUCTS(arena, group.index_count, GLindex);
		fread(group.indices, sizeof(GLindex), (size_t)group.index_count, objfile);
	}

	// TODO(kalle): Move the materials elsewhere
	fread(&mesh.material_count, sizeof(int), 1, objfile);

	mesh.materials = PUSH_STRUCTS(arena, mesh.material_count, MaterialData);
	for (i32 i = 0; i < mesh.material_count; ++i) {
		MaterialData &m = mesh.materials[i];

		fread(&m.Ns, sizeof(float), 4, objfile); // Ns to Tf
		fread(&m.illum, sizeof(u32), 1, objfile);
		fread(&m.Ka, sizeof(u32), 4, objfile); // Ka to Ke

		read_string(arena, m.map_Ka, objfile);
		read_string(arena, m.map_Kd, objfile);
		read_string(arena, m.map_Ks, objfile);
		read_string(arena, m.map_Ke, objfile);
		read_string(arena, m.map_d, objfile);
		read_string(arena, m.map_bump, objfile);
	}

	fclose(objfile);

	return mesh;
}

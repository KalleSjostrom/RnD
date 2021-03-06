#include "core/utils/string.h"

void read_string(ArenaAllocator &arena, String &string, FILE *file) {
	fread(&string.length, sizeof(i32), 1, file);
	string.text = (char*)allocate(&arena, string.length);
	fread(string.text, sizeof(char), (size_t)string.length, file);
}

MeshData read_obj(ArenaAllocator &arena, const char *filepath) {
	FILE *objfile;
	fopen_s(&objfile, filepath, "rb");
	ASSERT(objfile, "Could not open '%s'", filepath);

	MeshData mesh = {};

	fread(&mesh.vertex_count, sizeof(int), 1, objfile);
	fread(&mesh.coord_count, sizeof(int), 1, objfile);
	fread(&mesh.normal_count, sizeof(int), 1, objfile);
	fread(&mesh.group_count, sizeof(int), 1, objfile);

	mesh.vertices = PUSH(&arena, mesh.vertex_count, Vector3);
	mesh.coords = PUSH(&arena, mesh.coord_count, Vector2);
	mesh.normals = PUSH(&arena, mesh.normal_count, Vector3);
	mesh.groups = PUSH(&arena, mesh.group_count, GroupData);

	fread(mesh.vertices, sizeof(Vector3), (size_t)mesh.vertex_count, objfile);
	fread(mesh.coords, sizeof(Vector2), (size_t)mesh.coord_count, objfile);
	fread(mesh.normals, sizeof(Vector3), (size_t)mesh.normal_count, objfile);

	for (int i = 0; i < mesh.group_count; ++i) {
		GroupData &group = mesh.groups[i];
		fread(&group.material_index, sizeof(int), 1, objfile);
		fread(&group.index_count, sizeof(int), 1, objfile);
		group.indices = PUSH(&arena, group.index_count, GLindex);
		fread(group.indices, sizeof(GLindex), (size_t)group.index_count, objfile);
	}

	// TODO(kalle): Move the materials elsewhere
	fread(&mesh.material_count, sizeof(int), 1, objfile);

	mesh.materials = PUSH(&arena, mesh.material_count, MaterialData);
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

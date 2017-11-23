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

	fclose(objfile);

	return mesh;
}

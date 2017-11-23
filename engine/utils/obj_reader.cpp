MeshData read_obj(MemoryArena &arena, const char *filepath) {
	FILE *objfile;
	fopen_s(&objfile, filepath, "rb");
	ASSERT(objfile, "Could not open '%s'", filepath);

	MeshData mesh = {};

	int vertex_count;
	int coord_count;
	int normal_count;
	int group_count;

	fread(&vertex_count, sizeof(int), 1, objfile);
	fread(&coord_count, sizeof(int), 1, objfile);
	fread(&normal_count, sizeof(int), 1, objfile);
	fread(&group_count, sizeof(int), 1, objfile);

	mesh.vertices = PUSH_STRUCTS(arena, vertex_count, v3);
	mesh.coords = PUSH_STRUCTS(arena, coord_count, v2);
	mesh.normals = PUSH_STRUCTS(arena, normal_count, v3);

	GroupData *groups = PUSH_STRUCTS(arena, group_count, GroupData);

	for (int i = 0; i < group_count; ++i) {
		GroupData &group = groups[i];
		fread(&group.index_count, sizeof(int), 1, objfile);

		group.indices = PUSH_STRUCTS(arena, group.index_count, GLindex);
		fread(group.indices, sizeof(GLindex), (size_t)group.index_count, objfile);
	}

	fclose(objfile);

	return mesh;
}

struct MeshDataArray {
	MeshData *meshes;
	i32 count;
};

MeshDataArray read_obj(MemoryArena &arena, const char *filepath) {
	FILE *objfile;
	fopen_s(&objfile, filepath, "rb");
	ASSERT(objfile, "Could not open '%s'", filepath);

	int mesh_count;
	fread(&mesh_count, sizeof(int), 1, objfile);

	MeshData *meshes = PUSH_STRUCTS(arena, mesh_count, MeshData);

	for (int i = 0; i < mesh_count; ++i) {
		MeshData &o = meshes[i];

		fread(&o.vertex_count, sizeof(int), 1, objfile);
		fread(&o.coord_count, sizeof(int), 1, objfile);
		fread(&o.normal_count, sizeof(int), 1, objfile);
		fread(&o.index_count, sizeof(int), 1, objfile);

		o.vertices = PUSH_STRUCTS(arena, o.vertex_count, v3);
		o.coords = PUSH_STRUCTS(arena, o.coord_count, v2);
		o.normals = PUSH_STRUCTS(arena, o.normal_count, v3);
		o.indices = PUSH_STRUCTS(arena, o.index_count, GLindex);

		fread(o.vertices, sizeof(v3), o.vertex_count, objfile);
		fread(o.coords, sizeof(v2), o.coord_count, objfile);
		fread(o.normals, sizeof(v3), o.normal_count, objfile);
		fread(o.indices, sizeof(GLindex), o.index_count, objfile);
	}

	fclose(objfile);

	MeshDataArray data = {};
	data.meshes = meshes;
	data.count = mesh_count;

	return data;
}

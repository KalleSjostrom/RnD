#pragma pack()
struct ObjData {
	size_t vertex_count;
    size_t coord_count;
	size_t normal_count;
	size_t index_count;

	v3 *vertices;
	v2 *coords;
	v3 *normals;
	GLindex *indices;
};

ObjData read_obj(MemoryArena &arena, const char *filepath) {
	FILE *objfile;
	fopen_s(&objfile, filepath, "rb");
	ASSERT(objfile, "Could not open '%s'", filepath);

	ObjData o = {};

	fread(&o.vertex_count, sizeof(size_t), 1, objfile);
	fread(&o.coord_count, sizeof(size_t), 1, objfile);
    fread(&o.normal_count, sizeof(size_t), 1, objfile);
	fread(&o.index_count, sizeof(size_t), 1, objfile);

	o.vertices = PUSH_STRUCTS(arena, o.vertex_count, v3);
    o.coords = PUSH_STRUCTS(arena, o.coord_count, v2);
	o.normals = PUSH_STRUCTS(arena, o.normal_count, v3);
	o.indices = PUSH_STRUCTS(arena, o.index_count, GLindex);

	fread(o.vertices, sizeof(v3), o.vertex_count, objfile);
    fread(o.coords, sizeof(v2), o.coord_count, objfile);
    fread(o.normals, sizeof(v3), o.normal_count, objfile);
	fread(o.indices, sizeof(GLindex), o.index_count, objfile);

	fclose(objfile);

	return o;
}

struct Buffer {
	GLuint vbo;
	cl_mem mem;
};
inline v2 zero(int i) {
	return V2(0.0f, 0.0f);
}
inline v2 gen_random_pos(int i) {
	float x = (float)random()/RAND_MAX;
	x *= 9;
	float y = (float)random()/RAND_MAX;
	y *= 9;
	return V2(x, y);
}

inline v2 gen_pos(int i) {
	float x = -19.9f + (i % 120) * 0.126f * 1.3f * 0.8f;
	float y = -19.9f + (i / 120) * 0.126f * 1.3f * 0.8f;

	ASSERT(y <= 20, "Position out of bounds");
	ASSERT(x <= 20, "Position out of bounds");
	return V2(x, y);
}
Buffer gen_buffer(cl_context context, v2 (*f)(int i)) {
	Buffer buffer = {};
	glGenBuffers(1, &buffer.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
	v2 array[NR_PARTICLES];
	for (int i = 0; i < NR_PARTICLES; ++i) {
		array[i] = f(i);
	}
	glBufferData(GL_ARRAY_BUFFER, sizeof(v2)*NR_PARTICLES, array, GL_DYNAMIC_DRAW);

	cl_int errcode_ret;
	buffer.mem = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, buffer.vbo, &errcode_ret);
	CL_CHECK_ERRORCODE(clCreateFromGLBuffer, errcode_ret);

	return buffer;
}

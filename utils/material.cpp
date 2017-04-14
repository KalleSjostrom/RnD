namespace material {
	struct Buffer {
		int vao;
		int[] vbo;
	};

	public float[] model_view = new float[]{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1,
	};

	struct Material {
		int vertex_shader;
		int fragment_shader;
		int program;
		int projection_location;
		int model_view_location;
		int color_location;
		int alpha_location;
		Texture texture;
		Buffer buffer;
	};

	private static float[] SPRITE_BUFFER = new float[]{ 0, 0, 0, 1, 1, 1, 1, 0 };

	public void init(GL4 gl) {
		createProgram(gl, Shader.SPRITE);
		buffer = createBuffer(gl, SPRITE_BUFFER , SPRITE_BUFFER);
	}

	public void createProgram(GL4 gl, Shader shader) {
		program = glCreateProgram();

		vertex_shader = ShaderLibrary.getVertexShader(gl, shader);
		fragment_shader = ShaderLibrary.getFragmentShader(gl, shader);

		gl.glAttachShader(program, vertex_shader);
		gl.glAttachShader(program, fragment_shader);

		gl.glLinkProgram(program);
		gl.glUseProgram(program);

		projection_location = gl.glGetUniformLocation(program, "projection");
		model_view_location = gl.glGetUniformLocation(program, "model_view");
		color_location = gl.glGetUniformLocation(program, "color");
		alpha_location = gl.glGetUniformLocation(program, "alpha");

		gl.glUniformMatrix4fv(projection_location, 1, true, Values.ORTHOGRAPHIC_PROJECTION, 0);
		gl.glUniformMatrix4fv(model_view_location, 1, true, model_view, 0);

		gl.glUniform4fv(color_location, 1, new float[]{1, 1, 1, 1}, 0);
		// gl.glUniform4f(color_location, 1.0f, 1.0f, 1.0f, 1.0f);
		gl.glUniform1f(alpha_location, 1.0f);
	}

	public Buffer createBuffer(GL4 gl, float[] buffer_position, float[] buffer_uv) {
		int[] vbo = new int[2]; // position and uv
		gl.glGenBuffers(1, vbo, 0);
		gl.glBindBuffer(GL.GL_ARRAY_BUFFER, vbo[0]);
		gl.glBufferData(GL.GL_ARRAY_BUFFER, buffer_position.length * Buffers.SIZEOF_FLOAT, FloatBuffer.wrap(buffer_position), GL.GL_STATIC_DRAW);

		gl.glGenBuffers(1, vbo, 1);
		gl.glBindBuffer(GL.GL_ARRAY_BUFFER, vbo[1]);
		gl.glBufferData(GL.GL_ARRAY_BUFFER, buffer_uv.length * Buffers.SIZEOF_FLOAT, FloatBuffer.wrap(buffer_uv), GL.GL_STATIC_DRAW);

		int[] vao = new int[1]; // set of vertices (that can have the attributes position and uv).
		gl.glGenVertexArrays(1, vao, 0);
		gl.glBindVertexArray(vao[0]);

		gl.glBindBuffer(GL.GL_ARRAY_BUFFER, vbo[0]);
		int position_location = gl.glGetAttribLocation(program, "position");
		gl.glVertexAttribPointer(position_location, 2, GL.GL_FLOAT, false, 0, 0);
		gl.glEnableVertexAttribArray(0);

		gl.glBindBuffer(GL.GL_ARRAY_BUFFER, vbo[1]);
		int uv_location = gl.glGetAttribLocation(program, "vertex_uv");
		gl.glVertexAttribPointer(uv_location, 2, GL.GL_FLOAT, false, 0, 0);
		gl.glEnableVertexAttribArray(1);

		Buffer buffer = new Buffer();
		buffer.vao = vao;
		buffer.vbo = vbo;
		return buffer;
	}

	public Buffer refillBuffer(GL4 gl, Buffer buffer, float[] buffer_position, float[] buffer_uv) {
		gl.glBindBuffer(GL.GL_ARRAY_BUFFER, buffer.vbo[0]);
		gl.glBufferData(GL.GL_ARRAY_BUFFER, buffer_position.length * Buffers.SIZEOF_FLOAT, FloatBuffer.wrap(buffer_position), GL.GL_STATIC_DRAW);

		gl.glBindBuffer(GL.GL_ARRAY_BUFFER, buffer.vbo[1]);
		gl.glBufferData(GL.GL_ARRAY_BUFFER, buffer_uv.length * Buffers.SIZEOF_FLOAT, FloatBuffer.wrap(buffer_uv), GL.GL_STATIC_DRAW);

		return buffer;
	}


	public void destroy(GL4 gl) {
		gl.glDeleteBuffers(buffer.vbo.length, buffer.vbo, 0);

		gl.glDetachShader(program, vertex_shader);
		gl.glDetachShader(program, fragment_shader);

//		gl.glDeleteShader(vertex_shader);
//		gl.glDeleteShader(fragment_shader);

		gl.glDeleteProgram(program);
	}

	public void useMaterial(GL4 gl) {
		gl.glUseProgram(program);
		gl.glBindVertexArray(buffer.vao[0]);
	}

	public void render(GL4 gl) {
		// gl.glBlendFunc(GL.GL_ONE, GL.GL_ONE_MINUS_SRC_ALPHA);
		texture.bind(gl);

		gl.glDrawArrays(GL.GL_TRIANGLE_FAN, 0, 4);
		gl.glBindVertexArray(0);
	}

	public void setSprite(GL4 gl, Texture texture, float x, float y, float scale, float alpha) {
		this.texture = texture;

		model_view[3] = x;
		model_view[7] = y;
		model_view[0] = texture.getWidth() * scale;
		model_view[5] = texture.getHeight() * scale;

		gl.glUniformMatrix4fv(model_view_location, 1, true, model_view, 0);
		gl.glUniform1f(alpha_location, alpha);
	}
}

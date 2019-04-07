void update_texture(ImageData &image_data, GLuint texture) {
	glBindTexture(GL_TEXTURE_2D, texture);

	// The png is stored as ARGB
	// The tga is stored as BGR
	switch (image_data.format) {
		case PixelFormat_RGBA: {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_data.width, image_data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data.pixels);
		} break;
		case PixelFormat_ARGB: {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_data.width, image_data.height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, image_data.pixels);
		} break;
		case PixelFormat_RGB: {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_data.width, image_data.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, image_data.pixels);
		} break;
		case PixelFormat_BGR: {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_data.width, image_data.height, 0, GL_BGR, GL_UNSIGNED_BYTE, image_data.pixels);
		} break;
	}
}

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

GLuint load_texture(EngineApi *engine, String directory, String path, bool use_mipmap = true, GLuint default_texture = 0) {
	if (path.length == 0) {
		return default_texture;
	}

	ImageData image_data;
	char buf[1024] = {};
	int count = snprintf(buf, ARRAY_COUNT(buf), "%.*s/%.*s", (int)directory.length, *directory, (int)path.length, *path);
	for (int i = 0; i < count; ++i) {
		if (buf[i] == '\\')
			buf[i] = '/';
	}

	bool success = engine->image.load(buf, image_data);
	if (!success) {
		log_error("Model", "Could not load image '%s'!", buf);
		return default_texture;
	}

	GLuint texture;
	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (use_mipmap) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	update_texture(image_data, texture);

	if (use_mipmap) {
		glGenerateMipmap(GL_TEXTURE_2D); // This has to happen after we've sent the pixel-data to the card, i.e. after glTexImage2D
	}

	return texture;
}

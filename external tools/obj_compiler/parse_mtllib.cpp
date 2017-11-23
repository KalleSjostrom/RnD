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

struct Material {
	u32 name_id;

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

Material *parse_mtllib(MemoryArena &arena, const char *mtllib_filepath) {
	size_t filesize;
	FILE *file = open_file(mtllib_filepath, &filesize);

	char *source = (char *)PUSH_SIZE(arena, filesize + 1);
	fread(source, filesize, 1, file);
	source[filesize] = '\0';
	fclose(file);

	Material *materials = 0;
	array_init(materials, 32);

	parser::Tokenizer tok = parser::make_tokenizer(source, filesize, 0);
	parser::ParserContext pcontext;
	parser::set_parser_context(pcontext, &tok, (char*)mtllib_filepath);

	parser::Token t_newmtl   = TOKENIZE("newmtl");
	parser::Token t_Ns       = TOKENIZE("Ns");
	parser::Token t_Ni       = TOKENIZE("Ni");
	parser::Token t_d        = TOKENIZE("d");
	parser::Token t_Tr       = TOKENIZE("Tr");
	parser::Token t_Tf       = TOKENIZE("Tf");
	parser::Token t_illum    = TOKENIZE("illum");
	parser::Token t_Ka       = TOKENIZE("Ka");
	parser::Token t_Kd       = TOKENIZE("Kd");
	parser::Token t_Ks       = TOKENIZE("Ks");
	parser::Token t_Ke       = TOKENIZE("Ke");
	parser::Token t_map_Ka   = TOKENIZE("map_Ka");
	parser::Token t_map_Kd   = TOKENIZE("map_Kd");
	parser::Token t_map_d    = TOKENIZE("map_d");
	parser::Token t_map_bump = TOKENIZE("map_bump");
	parser::Token t_bump     = TOKENIZE("bump");

	bool parsing = true;
	while (parsing) {
		parser::Token token = parser::next_token(&tok);
		switch (token.type) {
			case TokenType_Identifier: {
				if (parser::is_equal(token, t_newmtl)) {
					array_new_entry(materials);

					token = parser::next_line(&tok);
					Material &material = array_last(materials);
					material.name_id = make_string_id32(token.string);
				} else if (parser::is_equal(token, t_Ns)) {
					array_last(materials).Ns = next_number(tok);
				} else if (parser::is_equal(token, t_Ni)) {
					array_last(materials).Ni = next_number(tok);
				} else if (parser::is_equal(token, t_d)) {
					array_last(materials).Tr = 1 - next_number(tok);
				} else if (parser::is_equal(token, t_Tr)) {
					array_last(materials).Tr = next_number(tok);
				} else if (parser::is_equal(token, t_Tf)) {
					array_last(materials).Tf = next_number(tok);
				} else if (parser::is_equal(token, t_illum)) {
					array_last(materials).illum = (IlluminationMode)(u32)next_number(tok);
				} else if (parser::is_equal(token, t_Ka)) {
					array_last(materials).Ka = V3(next_number(tok), next_number(tok), next_number(tok));
				} else if (parser::is_equal(token, t_Kd)) {
					array_last(materials).Kd = V3(next_number(tok), next_number(tok), next_number(tok));
				} else if (parser::is_equal(token, t_Ks)) {
					array_last(materials).Ks = V3(next_number(tok), next_number(tok), next_number(tok));
				} else if (parser::is_equal(token, t_Ke)) {
					array_last(materials).Ke = V3(next_number(tok), next_number(tok), next_number(tok));
				} else if (parser::is_equal(token, t_map_Ka)) {
					array_last(materials).map_Ka = clone_string(arena, token.string);
				} else if (parser::is_equal(token, t_map_Kd)) {
					array_last(materials).map_Kd = clone_string(arena, token.string);
				} else if (parser::is_equal(token, t_map_d)) {
					array_last(materials).map_d = clone_string(arena, token.string);
				} else if (parser::is_equal(token, t_map_bump)) {
					array_last(materials).map_bump = clone_string(arena, token.string);
				} else if (parser::is_equal(token, t_bump)) {
					array_last(materials).map_bump = clone_string(arena, token.string);
				}
			} break;
			case '#': {
				parser::consume_line(&tok);
			} break;
			case '\0': {
				parsing = false;
			} break;
		}
	}
	return materials;
}

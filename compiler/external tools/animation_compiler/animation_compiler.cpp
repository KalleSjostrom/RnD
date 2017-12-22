#include "utils/common.h"
#include "utils/parser.cpp"
#include "utils/math.h"

struct Keyframes {
	int count;
	m4 transforms[128];
	v3 world_positions[128];

	bool is_valid;
	m4 root;

	int parent_id;
};

struct AnimationData {
	String name;
	Keyframes keyframes[16];
};
struct AnimationDataArray {
	int count;
	AnimationData entries[1];
};

int extract_id(String &string) { // <animation id="joint3-anim" name="joint3">
	parser::fast_forward_until(&string.text, MAKE_STRING("name=\"joint"));

	for (int i = 0; i < string.length; i++) {
		if (!parser::is_numeric(string[i])) {
			string.text[i] = '\0';
			return atoi(string.text) - 1;
		}
	}
	return -1;
}

int extract_id(char *last) {
	char *end = last;
	char temp = end[0];

	last[0] = '\0';
	while (parser::is_numeric(*(--last)));

	int result = atoi(last+1) - 1;
	end[0] = temp;
	return result;
}

#define PARSE_NUMBER() parser::next_token(&tok, false).number

void parse_node(parser::Tokenizer &tok, AnimationData &animation, int id, int parent_id = -1) {
	parser::Token token = parser::next_token(&tok); // <matrix sid="matrix">

	Keyframes &keyframes = animation.keyframes[id];
	fill_m4(keyframes.root.m,
		PARSE_NUMBER(), PARSE_NUMBER(), PARSE_NUMBER(), PARSE_NUMBER(),
		PARSE_NUMBER(), PARSE_NUMBER(), PARSE_NUMBER(), PARSE_NUMBER(),
		PARSE_NUMBER(), PARSE_NUMBER(), PARSE_NUMBER(), PARSE_NUMBER(),
		PARSE_NUMBER(), PARSE_NUMBER(), PARSE_NUMBER(), PARSE_NUMBER()
	);
	keyframes.is_valid = true;
	keyframes.parent_id = parent_id;

	parser::fast_forward_until(&tok, MAKE_STRING("</extra>"));

	while (true) {
		token = parser::next_token(&tok); // <node name="joint2" id="joint2" sid="joint2" type="JOINT">
		if (starts_with(token.string, MAKE_STRING("node"))) {
			int child_id = extract_id(token.string);
			printf("Parent %d child %d\n", id, child_id);
			parse_node(tok, animation, child_id, id);
		} else {
			break;
		}
	}
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Invalid nr arguments %d\n", argc);
		fprintf(stderr, "Usage: animation_compiler infile.dae\n");
		return -1;
	}

	int bytes = 16*MB;
	MemoryArena arena = init_memory(bytes);
	MemoryArena data_arena = init_memory(bytes);

	AnimationDataArray animation_data_array = {};

	size_t filesize;
	FILE *dae_file = open_file(argv[1], &filesize);

	char *source = allocate_memory(arena, filesize+1);
	fread(source, 1, filesize, dae_file);
	source[filesize] = '\0';
	fclose(dae_file);

	parser::Tokenizer tok = parser::make_tokenizer(source, filesize);
	parser::Token token;

	AnimationData &animation = animation_data_array.entries[animation_data_array.count++];
	animation.name = MAKE_STRING("walk");

	String marker = MAKE_STRING("-Matrix-animation-output-transform-array");
	while (parser::fast_forward_until(&tok, marker)) {
		char *last = tok.at - marker.length;
		int node_id = extract_id(last);
		parser::fast_forward_until(&tok, '>');
		ASSERT_NEXT_TOKEN_TYPE(tok, '>');

		token = parser::next_token(&tok, false);

		while (token.type == TokenType_Number) {
			Keyframes &keyframes = animation.keyframes[node_id];
			m4 &mat = keyframes.transforms[keyframes.count++];
			fill_m4(mat.m,
				token.number,   PARSE_NUMBER(), PARSE_NUMBER(), PARSE_NUMBER(),
				PARSE_NUMBER(), PARSE_NUMBER(), PARSE_NUMBER(), PARSE_NUMBER(),
				PARSE_NUMBER(), PARSE_NUMBER(), PARSE_NUMBER(), PARSE_NUMBER(),
				PARSE_NUMBER(), PARSE_NUMBER(), PARSE_NUMBER(), PARSE_NUMBER()
			);

			token = parser::next_token(&tok, false);
		};
	}

	tok = parser::make_tokenizer(source, filesize);

	parser::fast_forward_until(&tok, MAKE_STRING("<library_visual_scenes>"));
	token = parser::next_token(&tok); // <node name="joint2" id="joint2" sid="joint2" type="JOINT">
	token = parser::next_token(&tok); // <visual_scene id="simple" name="simple">
	ASSERT(starts_with(token.string, MAKE_STRING("node")), "Unknown token");

	int root_id = extract_id(token.string);
	parse_node(tok, animation, root_id);

	FILE *output = fopen("../generated/animations.generated.cpp", "w");
	ASSERT(output, "No such file was found!");

fprintf(output, "namespace animation {\n");

fprintf(output, "	struct Data {\n");
fprintf(output, "		int keyframe_count;\n");
fprintf(output, "		v3 *keyframes;\n");
fprintf(output, "\n");
fprintf(output, "		int stride;\n");
fprintf(output, "	};\n");
fprintf(output, "	inline Data make_data(v3 *keyframes, int keyframe_count, int stride) {\n");
fprintf(output, "		Data d = { keyframe_count, keyframes, stride };\n");
fprintf(output, "		return d;\n");
fprintf(output, "	}\n");

	int max_count = 0;
	for (int i = 0; i < animation_data_array.count; ++i) {
		AnimationData &data = animation_data_array.entries[i];

		for (int j = 0; j < ARRAY_COUNT(data.keyframes); ++j) {
			Keyframes &keyframes = data.keyframes[j];
			if (!keyframes.is_valid)
				break;
			max_count = keyframes.count > max_count ? keyframes.count : max_count;
		}
	}

	for (int i = 0; i < animation_data_array.count; ++i) {
		AnimationData &data = animation_data_array.entries[i];

		for (int j = 0; j < ARRAY_COUNT(data.keyframes); ++j) {
			Keyframes &keyframes = data.keyframes[j];
			if (!keyframes.is_valid)
				break;

			if (keyframes.count == 0) { // Animation not present, need to generate it!
				if (j == 0) { // We are the root
					v3 root_position = translation(keyframes.root);
					for (int k = 0; k < max_count; ++k) {
						keyframes.transforms[keyframes.count++] = keyframes.root;
						keyframes.world_positions[k] = root_position;
					}
				} else {
					Keyframes &parent = data.keyframes[keyframes.parent_id];

					for (int k = 0; k < max_count; ++k) {
						keyframes.transforms[keyframes.count++] = parent.transforms[k] * keyframes.root; // Todo(kalle): make inplace
						keyframes.world_positions[k] = translation(keyframes.transforms[k]);
					}
				}
			} else {
				if (j == 0) { // We are the root
					for (int k = 0; k < keyframes.count; ++k) {
						m4 &transform = keyframes.transforms[k];
						keyframes.world_positions[k] = translation(transform);
					}
				} else {
					Keyframes &parent = data.keyframes[keyframes.parent_id];
					ASSERT(parent.count == keyframes.count, "");

					for (int k = 0; k < keyframes.count; ++k) {
						keyframes.transforms[k] = parent.transforms[k] * keyframes.transforms[k]; // Todo(kalle): make inplace
						keyframes.world_positions[k] = translation(keyframes.transforms[k]);
					}
				}
			}
		}
	}

	for (int i = 0; i < animation_data_array.count; ++i) {
		AnimationData &data = animation_data_array.entries[i];
fprintf(output, "	namespace %.*s {\n", data.name.length, *data.name);
fprintf(output, "		int stride = %d;\n", max_count);
fprintf(output, "		v3 keyframes[] = {\n");
		for (int j = 0; j < ARRAY_COUNT(data.keyframes); ++j) {
			Keyframes &keyframes = data.keyframes[j];
			if (keyframes.is_valid) {
// fprintf(output, "			{%f, %f, %f},\n", keyframes.root.x, keyframes.root.y, keyframes.root.z);
				for (int k = 0; k < keyframes.count; ++k) {
					v3 &p = keyframes.world_positions[k];
fprintf(output, "			{%f, %f, %f},\n", p.x, p.y, p.z);
				}
			}
		}
fprintf(output, "		};\n");
fprintf(output, "		Data data = make_data(keyframes, ARRAY_COUNT(keyframes), stride);\n");
fprintf(output, "	};\n");
	}
fprintf(output, "};\n");

	fclose(output);

	return 0;
}

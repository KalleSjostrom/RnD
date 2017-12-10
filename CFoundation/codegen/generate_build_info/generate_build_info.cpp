#include "../utils/common.cpp"

struct BuildInfo {
	unsigned rev;
	char node[32];
	char build_game_information[128];
};

#include "output_build_info.cpp"

const unsigned FILE_BUFFER_SIZE = 4*KB;

int main(int argc, char *argv[]) {
	// Read hg summary input
	char* build_info_input_path = "../../../tmp_build_info";
	MAKE_INPUT_FILE(input, build_info_input_path);

	// Get the file size
	fseek(input, 0, SEEK_END);
	unsigned size = ftell(input);
	fseek(input, 0, SEEK_SET);

	ASSERT(size < FILE_BUFFER_SIZE, "Build info input file size is larger than we have allocated!");

	// Read the contents
	char file_contents[FILE_BUFFER_SIZE] = {};
	fread(file_contents, sizeof(char), size, input);
	fclose(input);

	// Extract our wanted info (parent: 1865:9457d049c2a1)
	BuildInfo bi = {};
	sscanf(file_contents, "%*s%d:%s", &bi.rev, bi.node);

	// Generate some formatted strings
	sprintf(bi.build_game_information, "Game: %s (%d)", bi.node, bi.rev);

	output_build_info(bi);
}
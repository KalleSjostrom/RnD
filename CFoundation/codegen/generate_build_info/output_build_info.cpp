void output_build_info(BuildInfo  &build_info) {
	char* build_info_output_path = "../../"GAME_CODE_DIR"/generated/build_info.generated.h";
	MAKE_OUTPUT_FILE_WITH_HEADER(output, build_info_output_path);

fprintf(output, "namespace build_info {\n");
fprintf(output, "	static const char* BUILD_GAME_INFORMATION = \"%s\";\n", build_info.build_game_information);

	char tmp[64] = {};
fprintf(output, "	static const char* BUILD_GAME_REV_STR = \"%s\";\n", itoa(build_info.rev, tmp, 10));
fprintf(output, "	static const unsigned BUILD_GAME_REV_NUM = %d;\n", build_info.rev);

fprintf(output, "	static const char* BUILD_GAME_HASH = \"%s\";\n", build_info.node);
fprintf(output, "}");

	fclose(output);
}
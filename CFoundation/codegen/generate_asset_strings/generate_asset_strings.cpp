#include "../utils/common.cpp"

#define PROFILE 0
#if PROFILE
/*
time: 0.007270          iterate_content
time: 0.000048          iterate_packages
time: 0.007449          total
*/
#include "../utils/profiler.c"
enum ProfilerScopes {
	ProfilerScopes__iterate_content,
	ProfilerScopes__iterate_rendering,
	ProfilerScopes__iterate_packages,
	ProfilerScopes__total,
	ProfilerScopes__count,
};
#else
#define PROFILER_START(...)
#define PROFILER_STOP(...)
#define PROFILER_PRINT(...)
#endif

#define RELATIVE_PATH "../../../"
static const int relative_path_length = sizeof(RELATIVE_PATH)-1;
static const String _generated = MAKE_STRING("generated");

bool ignored_directory(String &directory) {
	return  are_strings_equal(directory, _generated);
}

bool accepted_file_ending(String &filename) {
	return 	string_ends_in(filename, MAKE_STRING(".entity")) ||
			string_ends_in(filename, MAKE_STRING(".unit")) ||
			string_ends_in(filename, MAKE_STRING(".shading_environment")) ||
			string_ends_in(filename, MAKE_STRING(".font")) ||
			string_ends_in(filename, MAKE_STRING(".package")) ||
			string_ends_in(filename, MAKE_STRING(".material")) ||
			string_ends_in(filename, MAKE_STRING(".particles"));
}

void write_indentation(FILE *output, unsigned indentation) {
	for (int i = 0; i < indentation; ++i) {
		fputc('\t', output);
	}
}

// HASH_COUNT is max number of assetcs with the accepted_file_ending, enough?
#define HASH_COUNT 4096

void iterate(FILE *output, uint64_t *asset_string_hash, char *filepath, unsigned indentation) {
	WIN32_FIND_DATA find_data = {};
	char next_filepath[MAX_PATH];
	sprintf(next_filepath, "%s*.*", filepath);

	static char variable_buffer[MAX_PATH] = {};

	HANDLE handle = find_first_file(next_filepath, &find_data);
	if (handle != INVALID_HANDLE_VALUE) {
		do {
			if (find_data.cFileName[0] == '.') {
				if (find_data.cFileName[1] == '\0')
					continue;

				if (find_data.cFileName[1] == '.' && find_data.cFileName[2] == '\0')
					continue;
			}

			String filename = make_string(find_data.cFileName);
			if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (!ignored_directory(filename) && filename[0] != '.') {
					sprintf(next_filepath, "%s%s/", filepath, *filename);

					write_indentation(output, indentation);

#if 1 // TODO(kalle): Remove this? cubemap folders contain a - character for some reason, this should not be so?
					for (int i = 0; i < filename.length; ++i) {
						if (filename[i] == '-') {
							variable_buffer[i] = '_';
						} else {
							variable_buffer[i] = filename[i];
						}
					}
					variable_buffer[filename.length] = '\0';
					fprintf(output, "namespace %s {\n", variable_buffer);
#else
					fprintf(output, "namespace %s {\n", *filename);
#endif

						iterate(output, asset_string_hash, next_filepath, indentation + 1);

					write_indentation(output, indentation);
					fprintf(output, "}\n");
				}
			} else if (accepted_file_ending(filename)) {
				unsigned variable_buffer_index = 0;
				for (unsigned i = 0; i < filename.length; i++) {
					if (filename[i] == '.' || filename[i] == '\0') {
						break;
					} else {
						variable_buffer[variable_buffer_index++] = filename[i];
					}
				}

				variable_buffer[variable_buffer_index++] = '\0';

				int written_size = sprintf(next_filepath, "%s%s", filepath, variable_buffer);
				char *path = next_filepath + relative_path_length; // Remove the initial dots

				uint64_t id64 = to_id64(written_size - relative_path_length, path);
				bool is_new_asset_path = true;
				unsigned index = id64 % HASH_COUNT;
				for (unsigned __i = 0; __i < HASH_COUNT; ++__i) {
					uint64_t entry = asset_string_hash[index];
					if (entry == 0) { // empty, insert us here
						asset_string_hash[index] = id64;
						break;
					} else if (entry == id64) { // found an entry with the same key as us, no need to print!
						is_new_asset_path = false;
						break;
					}
					// the entry is blocked by some other id64, try the next index
					index++;
					if (index == (HASH_COUNT))
						index = 0;
				}

				if (is_new_asset_path) {
					write_indentation(output, indentation);
					fprintf(output, "IdString64 %s = 0x%016llx; /* %s */\n", variable_buffer, id64, path);
				}
			}
		} while (FindNextFile(handle, &find_data));

		FindClose(handle);
	}
}

int main(int argc, char *argv[]) {
	MAKE_OUTPUT_FILE_WITH_HEADER(output, "../../"GAME_CODE_DIR"/generated/asset_strings.generated.cpp");

	size_t hash_size = HASH_COUNT * sizeof(uint64_t);
	MemoryArena arena = init_memory(hash_size);
	uint64_t *asset_string_hash = (uint64_t *)allocate_memory(arena, hash_size);
	memset(asset_string_hash, 0, hash_size);

PROFILER_START(total)
	// Parse the "content" folder
	PROFILER_START(iterate_content)
		fprintf(output, "namespace content {\n");
			iterate(output, asset_string_hash, RELATIVE_PATH"content/", 1);
		fprintf(output, "}\n");
	PROFILER_STOP(iterate_content)

	// Parse the "rendering" folder
	PROFILER_START(iterate_rendering)
		fprintf(output, "namespace rendering {\n");
			iterate(output, asset_string_hash, RELATIVE_PATH"rendering/", 1);
		fprintf(output, "}\n");
	PROFILER_STOP(iterate_rendering)

	// Parse the "packages" folder
	PROFILER_START(iterate_packages)
		fprintf(output, "namespace packages {\n");
			iterate(output, asset_string_hash, RELATIVE_PATH"packages/", 1);
		fprintf(output, "}\n");
	PROFILER_STOP(iterate_packages)
PROFILER_STOP(total)

	PROFILER_PRINT(iterate_content);
	PROFILER_PRINT(iterate_packages);
	PROFILER_PRINT(total);

	return 0;
}
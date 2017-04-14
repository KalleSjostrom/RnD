#include <stdio.h>
#include <stdlib.h>

namespace font {
	#pragma pack(push, 1)
	struct FontInfo {
		int ascent;
		int descent;
		int line_gap;
		int font_size;
		int num_chars;
	};

	struct FontCharacter {
		int xoff;
		int yoff;
		int width;
		int height;
		float left;
		float right;
		float bottom;
		float top;
		float xadvance;
	};
	#pragma pack(pop)

	struct Font {
		FontInfo info;
		FontCharacter *characters;
		int *kerning_table;
		int width;
		int height;
		unsigned char *pixels;
	};

	void load(MemoryArena &arena, const char *filename, Font *font) {
		FILE *font_file = fopen(filename, "rb");
		ASSERT_MSG_VAR(font_file, "No font file found at location '%s'\n", filename);

		fread(&font->info, sizeof(FontInfo), 1, font_file);

		font->characters = (FontCharacter *)allocate_memory(arena, font->info.num_chars * sizeof(FontCharacter));
		fread(font->characters, sizeof(FontCharacter), font->info.num_chars, font_file);

		int num_kernings = ('~' - ' ') + 1;
		num_kernings *= num_kernings; // squared

		font->kerning_table = (int *)allocate_memory(arena, sizeof(int) * num_kernings);
		fread(font->kerning_table, sizeof(int), num_kernings, font_file);

		int width;
		fread(&width, sizeof(int), 1, font_file);
		int height;
		fread(&height, sizeof(int), 1, font_file);
		font->width = width;
		font->height = height;

		int size = width * height;;

		font->pixels = (unsigned char *)malloc(size*sizeof(unsigned char));
		fread(font->pixels, sizeof(unsigned char), size, font_file);
	}

	inline int get_kerning(int *kerning_table, char a, char b) {
		int index = (a - ' ') + (b - ' ') * ('~' - ' ');
		return kerning_table[index];
	}
}
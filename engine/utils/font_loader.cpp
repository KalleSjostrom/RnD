#pragma once

#include <stdio.h>
#include <stdlib.h>

namespace font {
	#pragma pack(push, 1)
	struct FontInfo {
		i32 ascent;
		i32 descent;
		i32 line_gap;
		i32 font_size;
		i32 num_chars;
	};

	struct FontCharacter {
		i32 xoff;
		i32 yoff;
		i32 width;
		i32 height;
		f32 left;
		f32 right;
		f32 bottom;
		f32 top;
		f32 xadvance;
	};
	#pragma pack(pop)

	struct Font {
		FontInfo info;
		i32 __padding;
		FontCharacter *characters;
		i32 *kerning_table;
		i32 width;
		i32 height;
		unsigned char *pixels;
	};

	void load(MemoryArena &arena, const char *filename, Font *font) {
		FILE *file;
		fopen_s(&file, filename, "rb");
		ASSERT(file, "No font file found at location '%s'\n", filename);

		fread(&font->info, sizeof(FontInfo), 1, file);

		font->characters = PUSH_STRUCTS(arena, font->info.num_chars, FontCharacter);
		fread(font->characters, sizeof(FontCharacter), (u32)font->info.num_chars, file);

		i32 num_kernings = ('~' - ' ') + 1;
		num_kernings *= num_kernings; // squared

		font->kerning_table = PUSH_STRUCTS(arena, num_kernings, i32);
		fread(font->kerning_table, sizeof(i32), (u32)num_kernings, file);

		i32 width;
		fread(&width, sizeof(i32), 1, file);
		i32 height;
		fread(&height, sizeof(i32), 1, file);
		font->width = width;
		font->height = height;

		u32 size = (u32)(width * height);

		font->pixels = (unsigned char *)malloc(size*sizeof(unsigned char));
		fread(font->pixels, sizeof(unsigned char), size, file);
	}

	inline i32 get_kerning(i32 *kerning_table, char a, char b) {
		i32 index = (a - ' ') + (b - ' ') * ('~' - ' ');
		return kerning_table[index];
	}
}

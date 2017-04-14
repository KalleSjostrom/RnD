#include <stdio.h>

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"
#define STBRP_LARGE_RECTS
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define WIDTH 256
#define HEIGHT 256

int main(int argc, char **argv) {
	if (argc != 3) {
		fprintf(stderr, "Invalid nr arguments %d\n", argc);
		fprintf(stderr, "Usage: font_builder infile.ttf outfile.gamefont\n");
		return -1;
	}

	stbtt_pack_context spc;

	unsigned char *pixels = (unsigned char *)malloc(WIDTH*HEIGHT*sizeof(unsigned char));

	int stride_in_bytes = 0;
	int padding = 1;

	int result = stbtt_PackBegin(&spc, pixels, WIDTH, HEIGHT, stride_in_bytes, padding, NULL);

	unsigned int h_oversample = 1;
	unsigned int v_oversample = 1;
	stbtt_PackSetOversampling(&spc, h_oversample, v_oversample);

	FILE *ttf_file = fopen(argv[1], "rb");

	fseek(ttf_file, 0, SEEK_END);
	size_t length = ftell(ttf_file);
	fseek(ttf_file, 0, SEEK_SET);
	unsigned char *fontdata = (unsigned char *)malloc(length*sizeof(unsigned char));
	fread(fontdata, 1, length, ttf_file);
	fclose(ttf_file);

	int font_index = 0;
	int num_chars = ('~' - ' ')+1;
	stbtt_packedchar *chardata_for_range = (stbtt_packedchar*)malloc(num_chars*sizeof(stbtt_packedchar));

	stbtt_pack_range range;
	range.first_unicode_codepoint_in_range = ' ';
	range.array_of_unicode_codepoints = NULL;
	range.num_chars                   = num_chars;
	range.chardata_for_range          = chardata_for_range;
	range.font_size                   = 48;
	result = stbtt_PackFontRanges(&spc, fontdata, font_index, &range, 1);

	stbtt_PackEnd(&spc);


	FILE *file = fopen(argv[2], "wb");
	stbtt_fontinfo info;
	stbtt_InitFont(&info, fontdata, stbtt_GetFontOffsetForIndex(fontdata, font_index));

	int ascent, descent, line_gap;
	stbtt_GetFontVMetrics(&info, &ascent, &descent, &line_gap);

	int font_size = range.font_size;

	fwrite(&ascent, sizeof(ascent), 1, file);
	fwrite(&descent, sizeof(descent), 1, file);
	fwrite(&line_gap, sizeof(line_gap), 1, file);
	fwrite(&font_size, sizeof(font_size), 1, file);
	fwrite(&num_chars, sizeof(num_chars), 1, file);

	float inverse_width = 1.0f / WIDTH;
	float inverse_height = 1.0f / HEIGHT;

	for (int i = 0; i < num_chars; ++i) {
		stbtt_packedchar character = chardata_for_range[i];

		int xoff = character.xoff;
		int yoff = character.yoff;
		int width = character.x1-character.x0;
		int height = character.y1-character.y0;
		float x0 = character.x0*inverse_width;
		float x1 = character.x1*inverse_width;
		float y0 = character.y0*inverse_height;
		float y1 = character.y1*inverse_height;

		fwrite(&xoff,   sizeof(xoff),   1, file);
		fwrite(&yoff,   sizeof(yoff),   1, file);
		fwrite(&width,  sizeof(width),  1, file);
		fwrite(&height, sizeof(height), 1, file);
		fwrite(&x0,     sizeof(x0),     1, file);
		fwrite(&x1,     sizeof(x1),     1, file);
		fwrite(&y0,     sizeof(y0),     1, file);
		fwrite(&y1,     sizeof(y1),     1, file);

		float xadvance = character.xadvance;
		fwrite(&xadvance, sizeof(xadvance), 1, file);
	}

	for (char i = ' '; i <= '~'; ++i) {
		for (char j = ' '; j <= '~'; ++j) {
			int kern = stbtt_GetCodepointKernAdvance(&info, i, j);
			fwrite(&kern, sizeof(kern), 1, file);
		}
	}

	int width = WIDTH;
	int height = HEIGHT;
	int size = width * height;
	fwrite(&width, sizeof(width), 1, file);
	fwrite(&height, sizeof(height), 1, file);
	fwrite(pixels, sizeof(pixels[0]), size, file);

	fclose(file);

	free(pixels);
	return 0;
}

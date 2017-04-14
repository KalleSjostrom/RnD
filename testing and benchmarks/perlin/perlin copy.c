#include <stdio.h>
#include "immintrin.h"
#include "../../utils/profiler.c"

enum ProfilerScopes {
	ProfilerScopes__random_generation,

	ProfilerScopes__grad1,
	ProfilerScopes__grad2,
	ProfilerScopes__grad3,
	ProfilerScopes__grad4,
	ProfilerScopes__grad5,

	ProfilerScopes__count,
};

int p[] = {
	151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
	151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
	151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

inline float smooth(float t) {
	return t*t*t*(t*(t*6.0f - 15.0f) + 10.0f);
}

static float stb__perlin_grad(int hash, float x, float y, float z) {
	static float basis[12][3] =
	{
	  {  1, 1, 0 },
	  { -1, 1, 0 },
	  {  1,-1, 0 },
	  { -1,-1, 0 },
	  {  1, 0, 1 },
	  { -1, 0, 1 },
	  {  1, 0,-1 },
	  { -1, 0,-1 },
	  {  0, 1, 1 },
	  {  0,-1, 1 },
	  {  0, 1,-1 },
	  {  0,-1,-1 },
	};

	// perlin's gradient has 12 cases so some get used 1/16th of the time
	// and some 2/16ths. We reduce bias by changing those fractions
	// to 5/16ths and 6/16ths, and the same 4 cases get the extra weight.
	static unsigned char indices[64] =
	{
	  0,1,2,3,4,5,6,7,8,9,10,11,
	  0,9,1,11,
	  /*0,1,2,3,4,5,6,7,8,9,10,11,
	  0,1,2,3,4,5,6,7,8,9,10,11,
	  0,1,2,3,4,5,6,7,8,9,10,11,
	  0,1,2,3,4,5,6,7,8,9,10,11,*/
	};

   float *grad = basis[indices[hash & 15]];
   return grad[0]*x + grad[1]*y + grad[2]*z;
}

static float basis[36] = {
   1, 1, 0,
  -1, 1, 0,
   1,-1, 0,
  -1,-1, 0,
   1, 0, 1,
  -1, 0, 1,
   1, 0,-1,
  -1, 0,-1,
   0, 1, 1,
   0,-1, 1,
   0, 1,-1,
   0,-1,-1,
};
static unsigned char indices[16] = {
	0,1,2,3,4,5,6,7,8,9,10,11,
	0,9,1,11,
};
static float stb__perlin_grad2(int hash, float x, float y, float z) {
   float *grad = &basis[indices[hash & 15]*3];
   return grad[0]*x + grad[1]*y + grad[2]*z;
}


static float perlin_grad(int hash, float x, float y, float z) {
	int h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
	float u = h<8 ? x : y,
		v = h<4 ? y : h==12||h==14 ? x : z;                 // INTO 12 GRADIENT DIRECTIONS.
	return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}
static float perlin_grad2(int hash, float x, float y, float z) {
	int h = hash & 15;                                    // Take the hashed value and take the first 4 bits of it (15 == 0b1111)
	float u = h < 8 /* 0b1000 */ ? x : y;                // If the most significant bit (MSB) of the hash is 0 then set u = x.  Otherwise y.

	float v;                                             // In Ken Perlin's original implementation this was another conditional operator (?:).  I
	                                                      // expanded it for readability.

	if(h < 4 /* 0b0100 */)                                // If the first and second significant bits are 0 set v = y
	    v = y;
	else if(h == 12 /* 0b1100 */ || h == 14 /* 0b1110*/)  // If the first and second significant bits are 1 set v = x
	    v = x;
	else                                                  // If the first and second significant bits are not equal (0/1, 1/0) set v = z
	    v = z;

	return ((h&1) == 0 ? u : -u)+((h&2) == 0 ? v : -v); // Use the last 2 bits to decide if u and v are positive or negative.  Then return their addition.
}

static float grad(int hash, float x, float y, float z) {
	switch(hash & 0xF) {
		case 0x0: return  x + y;
		case 0x1: return -x + y;
		case 0x2: return  x - y;
		case 0x3: return -x - y;
		case 0x4: return  x + z;
		case 0x5: return -x + z;
		case 0x6: return  x - z;
		case 0x7: return -x - z;
		case 0x8: return  y + z;
		case 0x9: return -y + z;
		case 0xA: return  y - z;
		case 0xB: return -y - z;
		case 0xC: return  y + x;
		case 0xD: return -y + z;
		case 0xE: return  y - x;
		case 0xF: return -y - z;
		default: return 0; // never happens
	}
}

inline float lerp(float a, float b, float x) {
	return a + x * (b - a);
}

float perlin(float x, float y, float z) {
	int xi = (int)x; // & 255;								// Calculate the "unit cube" that the point asked will be located in
	int yi = (int)y; // & 255;								// The left bound is ( |_x_|,|_y_|,|_z_| ) and the right bound is that
	int zi = (int)z; // & 255;								// plus 1.  Next we calculate the location (from 0.0 to 1.0) in that cube.

	float xf = x-(int)x;								// We also smooth the location to smooth the result.
	float yf = y-(int)y;
	float zf = z-(int)z;

	float u = smooth(xf);
	float v = smooth(yf);
	float w = smooth(zf);

	int aaa = p[p[p[xi]+yi]+zi];
	int aba = p[p[p[xi]+yi+1]+zi];
	int aab = p[p[p[xi]+yi]+zi+1];
	int abb = p[p[p[xi]+yi+1]+zi+1];
	int baa = p[p[p[xi+1]+yi]+zi];
	int bba = p[p[p[xi+1]+yi+1]+zi];
	int bab = p[p[p[xi+1]+yi]+zi+1];
	int bbb = p[p[p[xi+1]+yi+1]+zi+1];

	float x1, x2, y1, y2;
	x1 = lerp(grad(aaa, xf, yf, zf),				// The gradient function calculates the dot product between a pseudorandom
			  grad(baa, xf-1, yf, zf),				// gradient vector and the vector from the input coordinate to the 8
				u);										// surrounding points in its unit cube.
	x2 = lerp(grad(aba, xf, yf-1, zf),				// This is all then lerped together as a sort of weighted average based on the faded (u,v,w)
			  grad(bba, xf-1, yf-1, zf),				// values we made earlier.
				  u);
	y1 = lerp(x1, x2, v);

	x1 = lerp(grad(aab, xf, yf, zf-1),
			  grad(bab, xf-1, yf, zf-1),
				u);
	x2 = lerp(grad(abb, xf, yf-1, zf-1),
			  grad(bbb, xf-1, yf-1, zf-1),
			  	u);
	y2 = lerp(x1, x2, v);

	return lerp(y1, y2, w);
}



static int stb__perlin_randtab[512] =
{
#if 0
   23, 125, 161, 52, 103, 117, 70, 37, 247, 101, 203, 169, 124, 126, 44, 123,
   152, 238, 145, 45, 171, 114, 253, 10, 192, 136, 4, 157, 249, 30, 35, 72,
   175, 63, 77, 90, 181, 16, 96, 111, 133, 104, 75, 162, 93, 56, 66, 240,
   8, 50, 84, 229, 49, 210, 173, 239, 141, 1, 87, 18, 2, 198, 143, 57,
   225, 160, 58, 217, 168, 206, 245, 204, 199, 6, 73, 60, 20, 230, 211, 233,
   94, 200, 88, 9, 74, 155, 33, 15, 219, 130, 226, 202, 83, 236, 42, 172,
   165, 218, 55, 222, 46, 107, 98, 154, 109, 67, 196, 178, 127, 158, 13, 243,
   65, 79, 166, 248, 25, 224, 115, 80, 68, 51, 184, 128, 232, 208, 151, 122,
   26, 212, 105, 43, 179, 213, 235, 148, 146, 89, 14, 195, 28, 78, 112, 76,
   250, 47, 24, 251, 140, 108, 186, 190, 228, 170, 183, 139, 39, 188, 244, 246,
   132, 48, 119, 144, 180, 138, 134, 193, 82, 182, 120, 121, 86, 220, 209, 3,
   91, 241, 149, 85, 205, 150, 113, 216, 31, 100, 41, 164, 177, 214, 153, 231,
   38, 71, 185, 174, 97, 201, 29, 95, 7, 92, 54, 254, 191, 118, 34, 221,
   131, 11, 163, 99, 234, 81, 227, 147, 156, 176, 17, 142, 69, 12, 110, 62,
   27, 255, 0, 194, 59, 116, 242, 252, 19, 21, 187, 53, 207, 129, 64, 135,
   61, 40, 167, 237, 102, 223, 106, 159, 197, 189, 215, 137, 36, 32, 22, 5,

   // and a second copy so we don't need an extra mask or static initializer
   23, 125, 161, 52, 103, 117, 70, 37, 247, 101, 203, 169, 124, 126, 44, 123,
   152, 238, 145, 45, 171, 114, 253, 10, 192, 136, 4, 157, 249, 30, 35, 72,
   175, 63, 77, 90, 181, 16, 96, 111, 133, 104, 75, 162, 93, 56, 66, 240,
   8, 50, 84, 229, 49, 210, 173, 239, 141, 1, 87, 18, 2, 198, 143, 57,
   225, 160, 58, 217, 168, 206, 245, 204, 199, 6, 73, 60, 20, 230, 211, 233,
   94, 200, 88, 9, 74, 155, 33, 15, 219, 130, 226, 202, 83, 236, 42, 172,
   165, 218, 55, 222, 46, 107, 98, 154, 109, 67, 196, 178, 127, 158, 13, 243,
   65, 79, 166, 248, 25, 224, 115, 80, 68, 51, 184, 128, 232, 208, 151, 122,
   26, 212, 105, 43, 179, 213, 235, 148, 146, 89, 14, 195, 28, 78, 112, 76,
   250, 47, 24, 251, 140, 108, 186, 190, 228, 170, 183, 139, 39, 188, 244, 246,
   132, 48, 119, 144, 180, 138, 134, 193, 82, 182, 120, 121, 86, 220, 209, 3,
   91, 241, 149, 85, 205, 150, 113, 216, 31, 100, 41, 164, 177, 214, 153, 231,
   38, 71, 185, 174, 97, 201, 29, 95, 7, 92, 54, 254, 191, 118, 34, 221,
   131, 11, 163, 99, 234, 81, 227, 147, 156, 176, 17, 142, 69, 12, 110, 62,
   27, 255, 0, 194, 59, 116, 242, 252, 19, 21, 187, 53, 207, 129, 64, 135,
   61, 40, 167, 237, 102, 223, 106, 159, 197, 189, 215, 137, 36, 32, 22, 5,
 #endif
	151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
	151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
};

static float stb__perlin_lerp(float a, float b, float t)
{
   return a + (b-a) * t;
}

#define stb__perlin_ease(a)   (((a*6-15)*a + 10) * a * a * a)

float stb_perlin_noise3(float x, float y, float z, int x_wrap, int y_wrap, int z_wrap)
{
   float u,v,w;
   float n000,n001,n010,n011,n100,n101,n110,n111;
   float n00,n01,n10,n11;
   float n0,n1;

   unsigned int x_mask = (x_wrap-1) & 255;
   unsigned int y_mask = (y_wrap-1) & 255;
   unsigned int z_mask = (z_wrap-1) & 255;

   int px = (int) x;
   int py = (int) y;
   int pz = (int) z;

   int x0 = px & x_mask, x1 = (px+1) & x_mask;
   int y0 = py & y_mask, y1 = (py+1) & y_mask;
   int z0 = pz & z_mask, z1 = (pz+1) & z_mask;

   x -= px; u = stb__perlin_ease(x);
   y -= py; v = stb__perlin_ease(y);
   z -= pz; w = stb__perlin_ease(z);

   int r0 = stb__perlin_randtab[x0];
   int r1 = stb__perlin_randtab[x1];

   int r00 = stb__perlin_randtab[r0+y0];
   int r01 = stb__perlin_randtab[r0+y1];
   int r10 = stb__perlin_randtab[r1+y0];
   int r11 = stb__perlin_randtab[r1+y1];

   n000 = stb__perlin_grad(stb__perlin_randtab[r00+z0], x  , y  , z   );
   n001 = stb__perlin_grad(stb__perlin_randtab[r00+z1], x  , y  , z-1 );
   n010 = stb__perlin_grad(stb__perlin_randtab[r01+z0], x  , y-1, z   );
   n011 = stb__perlin_grad(stb__perlin_randtab[r01+z1], x  , y-1, z-1 );
   n100 = stb__perlin_grad(stb__perlin_randtab[r10+z0], x-1, y  , z   );
   n101 = stb__perlin_grad(stb__perlin_randtab[r10+z1], x-1, y  , z-1 );
   n110 = stb__perlin_grad(stb__perlin_randtab[r11+z0], x-1, y-1, z   );
   n111 = stb__perlin_grad(stb__perlin_randtab[r11+z1], x-1, y-1, z-1 );

   n00 = stb__perlin_lerp(n000,n001,w);
   n01 = stb__perlin_lerp(n010,n011,w);
   n10 = stb__perlin_lerp(n100,n101,w);
   n11 = stb__perlin_lerp(n110,n111,w);

   n0 = stb__perlin_lerp(n00,n01,v);
   n1 = stb__perlin_lerp(n10,n11,v);

   return stb__perlin_lerp(n0,n1,u);
}



#define WIDTH 256
#define HEIGHT 256
#define SIZE (WIDTH * HEIGHT)

int main() {
	FILE *file = fopen("perlin.ppm", "w");
	fprintf(file, "P6\n");
	fprintf(file, "%i %i\n", WIDTH, HEIGHT);
	fprintf(file, "255\n");
	char *buf = (char *) calloc(SIZE*3, sizeof(char));

	int index = 0;
	for (int x = 0; x < WIDTH; x++) {
		for (int y = 0; y < HEIGHT; y++) {
			float p = perlin(x/20.0f, y/20.0f, 1.1f);
			float c = ((p+1)/2) * 255.0f;
			buf[index++] = c;
			buf[index++] = c;
			buf[index++] = c;
		}
	}

	fwrite(buf, 1, SIZE*3, file);
	fclose(file);
	free(buf);

	FILE *file2 = fopen("perlin2.ppm", "w");
	fprintf(file2, "P6\n");
	fprintf(file2, "%i %i\n", WIDTH, HEIGHT);
	fprintf(file2, "255\n");
	buf = (char *) calloc(SIZE*3, sizeof(char));

	index = 0;
	for (int x = 0; x < WIDTH; x++) {
		for (int y = 0; y < HEIGHT; y++) {
			float p = stb_perlin_noise3(x/20.0f, y/20.0f, 1.1f, 0, 0, 0);
			float c = ((p+1)/2) * 255.0f;
			buf[index++] = c;
			buf[index++] = c;
			buf[index++] = c;
		}
	}

	fwrite(buf, 1, SIZE*3, file2);
	fclose(file2);
	free(buf);

	int PROFILE_SIZE = 1024*1024*512;

	float *memory;
	posix_memalign((void*)&memory, 8 * sizeof(float), PROFILE_SIZE * sizeof(float));

	PROFILER_START(random_generation);
	for (int i = 0; i < PROFILE_SIZE; ++i) {
		memory[i] = i;
	}
	PROFILER_STOP_HITS(random_generation, PROFILE_SIZE);
	PROFILER_PRINT(random_generation);

	for (int i = 0; i < PROFILE_SIZE-2; ++i) {
		memory[i] = perlin_grad2(i, memory[i], memory[i+1], memory[+2]);
	}

	PROFILER_START(grad1);
	for (int i = 0; i < PROFILE_SIZE-2; ++i) {
		memory[i] = stb__perlin_grad(i, memory[i], memory[i+1], memory[+2]);
	}
	PROFILER_STOP_HITS(grad1, PROFILE_SIZE);
	PROFILER_PRINT(grad1);


	PROFILER_START(grad2);
	for (int i = 0; i < PROFILE_SIZE-2; ++i) {
		memory[i] = grad(i, memory[i], memory[i+1], memory[+2]);
	}
	PROFILER_STOP_HITS(grad2, PROFILE_SIZE);
	PROFILER_PRINT(grad2);


	PROFILER_START(grad3);
	for (int i = 0; i < PROFILE_SIZE-2; ++i) {
		memory[i] = perlin_grad(i, memory[i], memory[i+1], memory[+2]);
	}
	PROFILER_STOP_HITS(grad3, PROFILE_SIZE);
	PROFILER_PRINT(grad3);


	PROFILER_START(grad4);
	for (int i = 0; i < PROFILE_SIZE-2; ++i) {
		memory[i] = perlin_grad2(i, memory[i], memory[i+1], memory[+2]);
	}
	PROFILER_STOP_HITS(grad4, PROFILE_SIZE);
	PROFILER_PRINT(grad4);

	PROFILER_START(grad5);
	for (int i = 0; i < PROFILE_SIZE-2; ++i) {
		memory[i] = stb__perlin_grad2(i, memory[i], memory[i+1], memory[+2]);
	}
	PROFILER_STOP_HITS(grad5, PROFILE_SIZE);
	PROFILER_PRINT(grad5);

	return 0;
}


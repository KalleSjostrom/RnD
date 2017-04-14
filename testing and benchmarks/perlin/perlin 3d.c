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

#if USE_LOOKUP
static int random_lookup[] =
{
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
	// Duplicated to avoid overflow
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
#endif

static float basis[12][3] = {
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
static unsigned char indices[16] = {
	0,1,2,3,4,5,6,7,8,9,10,11,
	0,9,1,11,
};
static float gradient_dot(int hash, float x, float y, float z) {
	float *grad = basis[indices[hash & 15]];
	return grad[0]*x + grad[1]*y + grad[2]*z;
}

#define lerp(a, b, t) (a + (b-a) * t)
#define smooth(a) (((a*6-15)*a + 10) * a * a * a)

float perlin(float x, float y, float z)
{
	int x0 = (int)x;
	int y0 = (int)y;
	int z0 = (int)z;

	int x1 = (x0+1);
	int y1 = (y0+1);
	int z1 = (z0+1);

	float xf = x - x0;
	float yf = y - y0;
	float zf = z - z0;

	float u = smooth(xf);
	float v = smooth(yf);
	float w = smooth(zf);

#if USE_LOOKUP
	int r0 = random_lookup[x0];
	int r1 = random_lookup[x1];

	int r00 = random_lookup[r0+y0];
	int r01 = random_lookup[r0+y1];
	int r10 = random_lookup[r1+y0];
	int r11 = random_lookup[r1+y1];

	float n000 = gradient_dot(random_lookup[r00+z0], xf  , yf  , zf	);
	float n001 = gradient_dot(random_lookup[r00+z1], xf  , yf  , zf-1 );
	float n010 = gradient_dot(random_lookup[r01+z0], xf  , yf-1, zf	);
	float n011 = gradient_dot(random_lookup[r01+z1], xf  , yf-1, zf-1 );
	float n100 = gradient_dot(random_lookup[r10+z0], xf-1, yf  , zf	);
	float n101 = gradient_dot(random_lookup[r10+z1], xf-1, yf  , zf-1 );
	float n110 = gradient_dot(random_lookup[r11+z0], xf-1, yf-1, zf	);
	float n111 = gradient_dot(random_lookup[r11+z1], xf-1, yf-1, zf-1 );
#else
	// hash = (P1 ∗ cell.x xor P2 ∗ cell.y xor P3 ∗ cell.z) mod N
	#define P1 73856093
	#define P2 19349663
	#define P3 83492791

	float n000 = gradient_dot((P1*x0 ^ P2*y0 ^ P3*z0) % 255, xf  , yf  , zf	  );
	float n001 = gradient_dot((P1*x0 ^ P2*y0 ^ P3*z1) % 255, xf  , yf  , zf-1 );
	float n010 = gradient_dot((P1*x0 ^ P2*y1 ^ P3*z0) % 255, xf  , yf-1, zf	  );
	float n011 = gradient_dot((P1*x0 ^ P2*y1 ^ P3*z1) % 255, xf  , yf-1, zf-1 );
	float n100 = gradient_dot((P1*x1 ^ P2*y0 ^ P3*z0) % 255, xf-1, yf  , zf	  );
	float n101 = gradient_dot((P1*x1 ^ P2*y0 ^ P3*z1) % 255, xf-1, yf  , zf-1 );
	float n110 = gradient_dot((P1*x1 ^ P2*y1 ^ P3*z0) % 255, xf-1, yf-1, zf	  );
	float n111 = gradient_dot((P1*x1 ^ P2*y1 ^ P3*z1) % 255, xf-1, yf-1, zf-1 );
#endif

	float n00 = lerp(n000, n001, w);
	float n01 = lerp(n010, n011, w);
	float n10 = lerp(n100, n101, w);
	float n11 = lerp(n110, n111, w);

	float n0 = lerp(n00, n01, v);
	float n1 = lerp(n10, n11, v);

	return lerp(n0, n1, u);
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

	/*for (int i = 0; i < 256; ++i)
	{
		printf("%d, ", i);
	}*/

	return 0;
}


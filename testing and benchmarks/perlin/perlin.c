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

static double basis[4][2] = {
	{  1, 0 },
	{ -1, 0 },
	{  0, 1 },
	{  0,-1 },
};
static unsigned char indices[4] = {
	0,1,2,3,
};
static double gradient_dot(int hash, double x, double y) {
	double *grad = basis[indices[hash & 3]];
	return grad[0]*x + grad[1]*y;
}

#define lerp(a, b, t) (a + (b-a) * t)
#define smooth(a) (((a*6-15)*a + 10) * a * a * a)

inline double max(double a, double b, double c, double d) {
	double ab = a > b ? a : b;
	double cd = c > d ? c : d;
	return ab > cd ? ab : cd;
}

double perlin(double x, double y)
{
	int x0 = (int)x;
	int y0 = (int)y;

	int x1 = (x0+1);
	int y1 = (y0+1);

	double xf = x - x0;
	double yf = y - y0;

	double u = xf; // smooth(xf);
	double v = yf; // smooth(yf);

#if USE_LOOKUP
	int r0 = random_lookup[x0];
	int r1 = random_lookup[x1];

	double n00 = gradient_dot(random_lookup[r0+y0], xf  , yf  );
	double n01 = gradient_dot(random_lookup[r0+y1], xf  , yf-1);
	double n10 = gradient_dot(random_lookup[r1+y0], xf-1, yf  );
	double n11 = gradient_dot(random_lookup[r1+y1], xf-1, yf-1);
#else
	// hash = (P1 ∗ cell.x xor P2 ∗ cell.y xor P3 ∗ cell.z) mod N
	#define P1 73856093
	#define P2 19349663

/*
	double n00 = gradient_dot((P1*x0 ^ P2*y0) % 255, xf  , yf  );
	double n01 = gradient_dot((P1*x0 ^ P2*y1) % 255, xf  , yf-1);
	double n10 = gradient_dot((P1*x1 ^ P2*y0) % 255, xf-1, yf  );
	double n11 = gradient_dot((P1*x1 ^ P2*y1) % 255, xf-1, yf-1);*/

	double n00 = max((xf  ), (yf  ), -(xf  ), -(yf  ));
	double n01 = max((xf  ), (yf-1), -(xf  ), -(yf-1));
	double n10 = max((xf-1), (yf  ), -(xf-1), -(yf  ));
	double n11 = max((xf-1), (yf-1), -(xf-1), -(yf-1));
#endif

	double n0 = lerp(n00, n01, v);
	double n1 = lerp(n10, n11, v);

	// printf("u=%.3f, v=%.3f, n0=%.3f, n1=%.3f, n00=%.3f, n01=%.3f, n10=%.3f, n11=%.3f, xf=%.3f, yf=%.3f\n", u, v, n0, n1, n00, n01, n10, n11, xf, yf);
	// printf("%.3f, %.3f\n", n0, n1, u, v);

	return lerp(n0, n1, u);
}



#define WIDTH 256
#define HEIGHT 256
#define SIZE (WIDTH * HEIGHT)

int main() {
	/*FILE *file = fopen("perlin.ppm", "w");
	fprintf(file, "P6\n");
	fprintf(file, "%i %i\n", WIDTH, HEIGHT);
	fprintf(file, "255\n");
	char *buf = (char *) calloc(SIZE*3, sizeof(char));

	int index = 0;
	for (int x = 0; x < WIDTH; x++) {
		for (int y = 0; y < HEIGHT; y++) {
			double p = perlin(x/25.5f, y/25.5f);
			double c = ((p+1)/2) * 255.0f;
			buf[index++] = c;
			buf[index++] = c;
			buf[index++] = c;
		}
	}

	fwrite(buf, 1, SIZE*3, file);
	fclose(file);
	free(buf);*/


	/*printf("%.3f\n", perlin(-0.9999f, 0.9999f));
	printf("%.3f\n", perlin(0.9999f, -0.9999f));
	printf("%.3f\n", perlin(-0.9999f, -0.9999f));
	printf("%.3f\n", perlin(-0.5f, -0.5f));*/
	// printf("%.3f\n", perlin(0.0f, 0.0f));
	// printf("%.3f\n", perlin(0.25f, 0.25f));
	// printf("%.3f\n", perlin(0.25f, 0.5f));
	// printf("%.3f\n", perlin(0.5f, 0.25f));
	// printf("%.3f\n", perlin(0.5f, 0.5f));
	// printf("%.3f\n", perlin(0.75f, 0.5f));
	// printf("%.3f\n", perlin(0.75f, 0.75f));
	// printf("%.3f\n", perlin(0.9999f, 0.9999f));

	// printf("%.10f\n", perlin(0.5f, 1-0.70710678118f));
	printf("%.10f\n", perlin(0.5f, 0.70710678118f));
	printf("%.10f\n", perlin(0.6f, 0.70710678118f));
	printf("%.10f\n", perlin(0.999f, 0.999f));
	printf("%.10f\n", perlin(0.001f, 0.001f));
	printf("%.10f\n", perlin(0.001f, 0.999f));
	printf("%.10f\n", perlin(0.5f, 0.5f));
	printf("%.10f\n", perlin(0.5f, 0.999f));
	printf("%.10f\n", perlin(0.5f, 0.75f));

	printf("%.10f\n", perlin(0.5f, 0.80f));

	// cos(theta) = sqrt(2)/2
	// |a||b| * cos(45) > 0.70710678118f

	// printf("%.10f\n", perlin(1-0.70710678118f, 0.5f));
	// printf("%.10f\n", perlin(0.70710678118f, 0.5f));

	double max = -1000;
	double maxy, maxx;
	int size = 10240;
	for (int x = 0; x < size; ++x) {
		for (int y = 0; y < size; ++y) {
			double p = perlin(x/(double)size, y/(double)size);
			if (p > max) {
				max = p;
				maxx = x/(double)size;
				maxy = y/(double)size;
			}
		}
	}
	// [0.3491210938,0.5000000000] 0.5352764377
	printf("[%.10f,%.10f] %.10f\n", maxx, maxy, max);


	printf("%.10f\n", perlin(0.5f, 0.6508789062));

	printf("%.10f\n", smooth(0.640564f));
	printf("%.10f\n", smooth(0.6508789062));

	// Wolfram
	printf("%.10f\n", perlin(0.5f, 0.640564f));

	printf("%.10f\n", perlin(0.5f, 0.7661937076)); // 0.5622377638
	printf("%.10f\n", perlin(0.5f, 0.75)); // 0.5625000000

// 0.5352287292
// 0.5351409912


	/*for (int i = 0; i < 256; ++i)
	{
		printf("%d, ", i);
	}*/

	return 0;
}


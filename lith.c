#define _GNU_SOURCE
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#define LITH_SWAP(T, a, b) do { T t = a; a = b; b = t; } while (0)

#define FAILSAFE true

#define cosθ cos(θ)
#define sinθ sin(θ)

typedef float LtFloat;
typedef int   LtInt;
typedef void* LtPtr;

struct edge {
	LtInt a, b;
};

struct vec3i {
	LtInt x, y, z;
};

struct vec3 {
	LtFloat x, y, z;
};

struct vec2 {
	LtFloat x, y;
};

struct vec2i {
	LtInt x, y;
};

struct buffer {
	LtInt *fb;
};

// clockwise, a connects to b connects to c connects to a
struct triangle {
	// indecies of vec3s in an array
	LtInt a, b, c;
};

struct mesh {
	struct edge *verts;
	struct vec3 *vects;
	struct triangle *tris;
	LtInt am, ram, tam;
};

enum error {
	LT_ALLOC,
	LT_BOUNDS,
};

typedef struct buffer   buffer;
typedef struct vec2     vec2;
typedef struct vec3     vec3;
typedef struct vec2i    vec2i;
typedef struct vec3i    vec3i;
typedef struct triangle triangle;
typedef struct edge     edge;
typedef struct mesh     mesh;
typedef enum   error    error;

LtInt   LtWidth       = 0;
LtInt   LtHeight      = 0;
LtFloat LtFocalLength = 0;
LtPtr   LtAllocs[65536] = { 0 };
LtInt   LtNewestAlloc = 0;

vec2i ltNDCToScreen(vec2 v, LtInt w, LtInt h)
{
	vec2i screenCoord;
	screenCoord.x = (LtInt) ((v.x + 1.0f) * 0.5f * w);
	screenCoord.y = (LtInt) ((1.0f - v.y) * 0.5f * h);
	screenCoord.x = screenCoord.x >= w ? w - 1 : screenCoord.x;
	screenCoord.y = screenCoord.y >= h ? h - 1 : screenCoord.y;
	screenCoord.x = screenCoord.x < 0 ? 1 : screenCoord.x;
	screenCoord.y = screenCoord.y < 0 ? 1 : screenCoord.y;
	return screenCoord;
}

vec3 ltScale3Dvec(vec3 v, LtFloat s)
{
	return (vec3) {v.x * s, v.y * s, v.z * s};
}

vec3 ltAdd3Dvecs(vec3 v0, vec3 v1)
{
	return (vec3) {v0.x + v1.x, v0.y + v1.y, v0.z + v1.z};
}

vec3 ltSub3Dvecs(vec3 v0, vec3 v1)
{
	return (vec3) {v0.x - v1.x, v0.y - v1.y, v0.z - v1.z};
}

vec2i ltProject(vec3 p, LtFloat foc, LtInt w, LtInt h, vec3 cam)
{
	vec3 view = ltSub3Dvecs(p, cam);

	LtFloat pX = (view.x * foc) / fmax(view.z + foc - 0.02f, 0.01f);
	LtFloat pY = (view.y * foc) / fmax(view.z + foc - 0.02f, 0.01f);

	vec2 pr = (vec2) {pX, pY};

	return ltNDCToScreen(pr, w, h);
}

void ltTransform(mesh *m, void (*f)(vec3 *))
{
	for (LtInt i = 0; i < m->am; i++)
		f(&m->vects[i]);
}

void ltClear(buffer *buf, LtInt col)
{
	for (LtInt i = 0; i < LtWidth * LtHeight; i++)
		buf->fb[i] = col;
}

void ltDrawPixel(vec2i p, LtInt col, buffer *b)
{
	b->fb[p.x + p.y * LtWidth] = col;
}

void ltDrawLine(vec2i v1, vec2i v2, LtInt col, buffer *b)
{
	LtInt w = LtWidth; LtInt h = LtHeight;

	LtInt x0 = v1.x, y0 = v1.y;
	LtInt x1 = v2.x, y1 = v2.y;

	LtInt dx = abs(x1 - x0);
	LtInt dy = abs(y1 - y0);

	LtInt sx = (x0 < x1) ? 1 : -1;
	LtInt sy = (y0 < y1) ? 1 : -1;

	LtInt err = dx - dy;

	while (1) {
		if (x0 >= 0 && y0 >= 0 && x0 < w && y0 < h)
			ltDrawPixel((vec2i) {x0, y0}, col, b);
		else
			return;
		
		if (x0 == x1 && y0 == y1) break;
		
		LtInt e2 = 2 * err;
	
		if (e2 > -dy) {
			err -= dy;
			x0 += sx;
		}
		
		if (e2 < dx) {
			err += dx;
			y0 += sy;
		}
	}

	return;
}

void ltDrawTriangle(triangle tri, vec3 *points, buffer *buf, LtInt col, vec3 cam)
{
	vec2i a, b, c;

	a = ltProject(points[tri.a], LtFocalLength, LtWidth, LtHeight, cam);
	b = ltProject(points[tri.b], LtFocalLength, LtWidth, LtHeight, cam);
	c = ltProject(points[tri.c], LtFocalLength, LtWidth, LtHeight, cam);

	if (FAILSAFE) {
		ltDrawLine(a, b, col, buf);
		ltDrawLine(b, c, col, buf);
		ltDrawLine(c, a, col, buf);
	}

	int y0, y1, y2, x0, x1, x2;
	y0 = a.y;y1 = b.y;y2 = c.y;
	x0 = a.x;x1 = b.x;x2 = c.x;

	if (y0 > y1) { LITH_SWAP(int, y0, y1); LITH_SWAP(int, x0, x1); }
	if (y0 > y2) { LITH_SWAP(int, y0, y2); LITH_SWAP(int, x0, x2); }
	if (y1 > y2) { LITH_SWAP(int, y1, y2); LITH_SWAP(int, x1, x2); }

	int total_h = y2 - y0;

	for (int i = 0; i < total_h; i++) {
		bool second_half = i > y1 - y0 || y1 == y0;
		int segment_h = second_half ? y2 - y1 : y1 - y0;
		float alpha = (float) i / total_h;
		float beta = (float) (i - (second_half ? y1 - y0 : 0)) / segment_h;
		int ax = x0 + (x2 - x0) * alpha;
		int ay = y0 + i;
		int bx = second_half ? x1 + (x2 - x1) * beta : x0 + (x1 - x0) * beta;
		int by = y0 + i;

		if (ax > bx) {
			LITH_SWAP(int, ax, bx);
			LITH_SWAP(int, ay, by);
		}

		for (int j = ax; j <= bx; j++)
			if (0 <= j && j < LtWidth && 0 <= ay && ay < LtHeight)
				ltDrawPixel((vec2i) {j, ay}, col, buf);
	}
}

void ltDrawTriangles(triangle *tris, vec3 *points, buffer *buf, LtInt *cols, LtInt tricount, vec3 cam)
{
	for (int i = 0; i < tricount; i++)
		ltDrawTriangle(tris[i], points, buf, cols[i], cam);
}

void ltRenderMesh(LtInt col, buffer *buf, edge *edges, vec3 *points, LtInt vert_am, vec3 cam)
{
	LtInt   w   = LtWidth;
	LtInt   h   = LtHeight;
	LtFloat foc = LtFocalLength;

	for (LtInt i = 0; i < vert_am; i++) {
		vec3 a = points[edges[i].a];
		vec3 b = points[edges[i].b];

		vec2i Pa = ltProject(a, foc, w, h, cam);
		vec2i Pb = ltProject(b, foc, w, h, cam);

		ltDrawLine(Pa, Pb, col, buf);
	}
}

void ltPrintb(buffer *b)
{
	LtInt w = LtWidth;
	LtInt h = LtHeight;

	for (LtInt y = 0; y < h; y++) {
		for (LtInt x = 0; x < w; x++)
			printf("%d", b->fb[y * w + x]);
		putchar('\n');
	}
}

vec3 ltRotateX(vec3 in, LtFloat θ)
{
	LtFloat x´, y´, z´, x, y, z;
	x = in.x; y = in.y; z = in.z;

	x´ = x;
	y´ = y * cosθ - z * sinθ;
	z´ = y * sinθ + z * cosθ;


	return (vec3) {x´, y´, z´};
}

vec3 ltRotateY(vec3 in, LtFloat θ)
{
	LtFloat x´, y´, z´, x, y, z;
	x = in.x; y = in.y; z = in.z;

	x´ = x * cosθ - z * sinθ;
	y´ = y;
	z´ = x * sinθ + z * cosθ;

	return (vec3) {x´, y´, z´};
}

vec3 ltRotateZ(vec3 in, LtFloat θ)
{
	LtFloat x´, y´, z´, x, y, z;
	x = in.x; y = in.y; z = in.z;

	x´ = x * cosθ - y * sinθ;
	y´ = x * sinθ + y * cosθ;
	z´ = z;

	return (vec3) {x´, y´, z´};
}

LtFloat ltDegreeToRads(LtFloat deg)
{
	return deg * (M_PI / 180);
}

void ltPushAlloc(void *alloc)
{
	LtAllocs[LtNewestAlloc] = alloc;
	LtNewestAlloc++;
}

//void ltSaveLTObjectFile(string path, mesh mesh)
//{
//	ofstream file(path, ios::binary);
//
//	file << "LTObjF\n";
//	file << "vects\n";
//	
//	for (auto &vec : mesh.vects) {
//		vector<char> th(3 * sizeof(LtFloat), 0);
//
//		memcpy(&th[0], &vec.x, sizeof(LtFloat));
//		memcpy(&th[4], &vec.y, sizeof(LtFloat));
//		memcpy(&th[8], &vec.z, sizeof(LtFloat));
//
//		file.write(reLtInterpret_cast<char*> (th.data()), th.size());
//		file << "\n";
//	}
//
//	file << "verts\n";
//	
//	for (auto &ver : mesh.verts) {
//		vector<char> th(2 * sizeof(LtInt), 0);
//
//		memcpy(&th[0], &ver.a, sizeof(LtInt));
//		memcpy(&th[4], &ver.b, sizeof(LtInt));
//
//		file.write(reLtInterpret_cast<char*> (th.data()), th.size());
//		file << "\n";
//	}
//}

void ltHeapDebug(void)
{
	printf("Allocs: %d\n", LtNewestAlloc);
}

void ltLogError(error err, char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);

	ltHeapDebug();
	vfprintf(stderr, fmt, arg);
	putc('\n', stderr);
	fprintf(stderr, "Failure reason: %d\n", err);

	va_end(arg);
}

mesh ltReadLTObjectFile(char *path)
{
	mesh Mesh;
	Mesh.am = 0;
	Mesh.ram = 0;
	FILE *file = fopen(path, "r");

	Mesh.verts = (edge *) calloc(256, sizeof(edge));
	Mesh.vects = (vec3 *) calloc(256, sizeof(vec3));

	if (!Mesh.verts || !Mesh.vects) {
		ltLogError(LT_ALLOC, "Allocation error");
		abort();
	}

	ltPushAlloc(Mesh.verts);
	ltPushAlloc(Mesh.vects);

	char *line = (char *) calloc(1024, 1);

	LtInt mode = 0; // 0 is invalid
		      // 1 is poLtInts/vectors
		      // 2 is edges (called verts)

	LtInt len = 0;

	fgets(line, 16, file);

	if (strcmp(line, "LTObjF\n")) return Mesh;

	while (fgets(line, 16, file)) {
		if (!strcmp(line, "\n")) continue;

		if (!strcmp(line, "vects\n")) {
			mode = 1;
			continue;
		} else if (!strcmp(line, "verts\n")) {
			mode = 2;
			continue;
		}

		if (mode == 1) {
			LtFloat x, y, z;
			char *floatX = &line[0];
			char *floatY = &line[4];
			char *floatZ = &line[8];
			memcpy(&x, floatX, sizeof(LtFloat));
			memcpy(&y, floatY, sizeof(LtFloat));
			memcpy(&z, floatZ, sizeof(LtFloat));

			Mesh.vects[Mesh.am] = (vec3) {x, y, z};
			Mesh.am++;
		} else if (mode == 2) {
			LtInt a, b;
			char *intA = &line[0];
			char *intB = &line[4];
			memcpy(&a, intA, sizeof(LtInt));
			memcpy(&b, intB, sizeof(LtInt));

			Mesh.verts[Mesh.ram] = (edge) {a, b};
			Mesh.ram++;
		}

	}

	fclose(file);
	free(line);

	return Mesh;
}

buffer ltInitBuffer(void)
{
	buffer a;

	a.fb = (LtInt *) calloc(LtWidth * LtHeight, sizeof(LtInt));

	LtAllocs[LtNewestAlloc] = a.fb;
	LtNewestAlloc++;

	return a;
}

void ltInit(LtFloat foc, LtInt w, LtInt h)
{
	LtHeight = h; LtWidth = w; LtFocalLength = foc;
}

void ltTerminate(void)
{
	for (LtInt i = 0; i < LtNewestAlloc; i++)
		free(LtAllocs[i]);
}

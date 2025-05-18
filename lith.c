#define _GNU_SOURCE
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define cosθ cos(θ)
#define sinθ sin(θ)

struct Vertex {
	int a, b;
};

struct Point3i {
	int x, y, z;
};

struct Point3 {
	float x, y, z;
};

struct Point2 {
	float x, y;
};

struct Point2i {
	int x, y;
};

struct Buffer {
	int *fb;
};

struct Mesh {
	struct Vertex *verts;
	struct Point3 *vects;
	int am, ram;
};

enum Error {
	LT_ALLOC,
	LT_BOUNDS,
};

#define Vector3 Point3
#define Vector2 Point2

typedef struct Buffer  Buffer;
typedef struct Point2  Point2;
typedef struct Point3  Point3;
typedef struct Point2i Point2i;
typedef struct Point3i Point3i;
typedef struct Vertex  Vertex;
typedef struct Vector2 Vector2;
typedef struct Vector3 Vector3;
typedef struct Mesh    Mesh;
typedef struct Buffer  Buffer;
typedef enum   Error   Error;

typedef struct Buffer  LtBuffer;
typedef struct Point2  LtPoint2;
typedef struct Point3  LtPoint3;
typedef struct Point2i LtPoint2i;
typedef struct Point3i LtPoint3i;
typedef struct Vertex  LtVertex;
typedef struct Vector2 LtVector2;
typedef struct Vector3 LtVector3;
typedef struct Mesh    LtMesh;
typedef enum   Error   LtError;

typedef float LtFloat;
typedef int   LtInt;
typedef void* LtPtr;

LtInt   LtWidth       = 0;
LtInt   LtHeight      = 0;
LtFloat LtFocalLength = 0;
LtPtr   LtAllocs[65536] = { 0 };
LtInt   LtNewestAlloc = 0;

Point2i ltNDCToScreen(Point2 v, int w, int h)
{
	Point2i screenCoord;
	screenCoord.x = (int) ((v.x + 1.0f) * 0.5f * w);
	screenCoord.y = (int) ((1.0f - v.y) * 0.5f * h);
	screenCoord.x = screenCoord.x >= w ? w - 1 : screenCoord.x;
	screenCoord.y = screenCoord.y >= h ? h - 1 : screenCoord.y;
	screenCoord.x = screenCoord.x < 0 ? 1 : screenCoord.x;
	screenCoord.y = screenCoord.y < 0 ? 1 : screenCoord.y;
	return screenCoord;
}

Point3 ltScale3DVector(Point3 v, float s)
{
	return (Point3) {v.x * s, v.y * s, v.z * s};
}

Point3 ltAdd3DVectors(Point3 v0, Point3 v1)
{
	return (Point3) {v0.x + v1.x, v0.y + v1.y, v0.z + v1.z};
}

Point3 ltSub3DVectors(Point3 v0, Point3 v1)
{
	return (Point3) {v0.x - v1.x, v0.y - v1.y, v0.z - v1.z};
}

Point2i ltProject(Point3 p, float foc, int w, int h, Point3 cam)
{
	Point3 view = ltSub3DVectors(p, cam);

	float pX = (view.x * foc) / fmax(view.z + foc - 0.02f, 0.01f);
	float pY = (view.y * foc) / fmax(view.z + foc - 0.02f, 0.01f);

	Point2 pr = (Point2) {pX, pY};

	return ltNDCToScreen(pr, w, h);
}

void ltDrawPixel(Point2i p, int col, Buffer *b, int w, int h)
{
	b->fb[p.x + p.y * w] = col;
}

void ltDrawLine(Point2i v1, Point2i v2, int col, Buffer *b, int w, int h)
{
	int x0 = v1.x, y0 = v1.y;
	int x1 = v2.x, y1 = v2.y;

	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);

	int sx = (x0 < x1) ? 1 : -1;
	int sy = (y0 < y1) ? 1 : -1;

	int err = dx - dy;

	while (1) {
		if (x0 >= 0 && y0 >= 0 && x0 < w && y0 < h)
			ltDrawPixel((Point2i) {x0, y0}, col, b, w, h);
		else
			return;
		
		if (x0 == x1 && y0 == y1) break;
		
		int e2 = 2 * err;
	
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

void ltRenderMesh(int col, Buffer *buf, Vertex *vertices, Point3 *points, LtInt vert_am, Point3 cam)
{
	LtInt   w   = LtWidth;
	LtInt   h   = LtHeight;
	LtFloat foc = LtFocalLength;

	for (int i = 0; i < vert_am; i++) {
		Point3 a = points[vertices[i].a];
		Point3 b = points[vertices[i].b];

		Point2i Pa = ltProject(a, foc, w, h, cam);
		Point2i Pb = ltProject(b, foc, w, h, cam);

		ltDrawLine(Pa, Pb, col, buf, w, h);
	}
}

void ltPrintb(Buffer *b)
{
	LtInt w = LtWidth;
	LtInt h = LtHeight;

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++)
			printf("%d", b->fb[y * w + x]);
		putchar('\n');
	}
}

Point3 ltRotateX(Point3 in, float θ)
{
	float x´, y´, z´, x, y, z;
	x = in.x; y = in.y; z = in.z;

	x´ = x;
	y´ = y * cosθ - z * sinθ;
	z´ = y * sinθ + z * cosθ;


	return (Point3) {x´, y´, z´};
}

Point3 ltRotateY(Point3 in, float θ)
{
	float x´, y´, z´, x, y, z;
	x = in.x; y = in.y; z = in.z;

	x´ = x * cosθ - z * sinθ;
	y´ = y;
	z´ = x * sinθ + z * cosθ;

	return (Point3) {x´, y´, z´};
}

Point3 ltRotateZ(Point3 in, float θ)
{
	float x´, y´, z´, x, y, z;
	x = in.x; y = in.y; z = in.z;

	x´ = x * cosθ - y * sinθ;
	y´ = x * sinθ + y * cosθ;
	z´ = z;

	return (Point3) {x´, y´, z´};
}

float ltDegreeToRads(float deg)
{
	return deg * (M_PI / 180);
}

void ltPushAlloc(void *alloc)
{
	LtAllocs[LtNewestAlloc] = alloc;
	LtNewestAlloc++;
}

//void ltSaveLTObjectFile(string path, Mesh mesh)
//{
//	ofstream file(path, ios::binary);
//
//	file << "LTObjF\n";
//	file << "vects\n";
//	
//	for (auto &vec : mesh.vects) {
//		vector<char> th(3 * sizeof(float), 0);
//
//		memcpy(&th[0], &vec.x, sizeof(float));
//		memcpy(&th[4], &vec.y, sizeof(float));
//		memcpy(&th[8], &vec.z, sizeof(float));
//
//		file.write(reinterpret_cast<char*> (th.data()), th.size());
//		file << "\n";
//	}
//
//	file << "verts\n";
//	
//	for (auto &ver : mesh.verts) {
//		vector<char> th(2 * sizeof(int), 0);
//
//		memcpy(&th[0], &ver.a, sizeof(int));
//		memcpy(&th[4], &ver.b, sizeof(int));
//
//		file.write(reinterpret_cast<char*> (th.data()), th.size());
//		file << "\n";
//	}
//}

void ltHeapDebug(void)
{
	printf("Allocs: %d\n", LtNewestAlloc);
}

void ltLogError(LtError err, char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);

	ltHeapDebug();
	vfprintf(stderr, fmt, arg);
	putc('\n', stderr);
	fprintf(stderr, "Failure reason: %d\n", err);

	va_end(arg);
}

Mesh ltReadLTObjectFile(char *path)
{
	Mesh mesh;
	mesh.am = 0;
	mesh.ram = 0;
	FILE *file = fopen(path, "r");

	mesh.verts = (Vertex *) calloc(256, sizeof(Vertex));
	mesh.vects = (Vector3 *) calloc(256, sizeof(Vector3));

	if (!mesh.verts || !mesh.vects) {
		ltLogError(LT_ALLOC, "Allocation error");
		abort();
	}

	ltPushAlloc(mesh.verts);
	ltPushAlloc(mesh.vects);

	char *line = (char *) calloc(1024, 1);

	int mode = 0; // 0 is invalid
		      // 1 is points/vectors
		      // 2 is vertices

	int len = 0;

	fgets(line, 16, file);

	if (strcmp(line, "LTObjF\n")) return mesh;

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
			float x, y, z;
			char *floatX = &line[0];
			char *floatY = &line[4];
			char *floatZ = &line[8];
			memcpy(&x, floatX, sizeof(float));
			memcpy(&y, floatY, sizeof(float));
			memcpy(&z, floatZ, sizeof(float));

			mesh.vects[mesh.am] = (Point3) {x, y, z};
			mesh.am++;
		} else if (mode == 2) {
			int a, b;
			char *intA = &line[0];
			char *intB = &line[4];
			memcpy(&a, intA, sizeof(int));
			memcpy(&b, intB, sizeof(int));

			mesh.verts[mesh.ram] = (Vertex) {a, b};

			mesh.ram++;
		}

	}

	printf("Loaded %d Vectors (Points) and %d Vertices (Edges)\n", mesh.am, mesh.ram);

	fclose(file);
	free(line);

	return mesh;
}

Buffer ltInitBuffer(void)
{
	Buffer a;

	a.fb = (int *) calloc(LtWidth * LtHeight, sizeof(int));

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
	for (int i = 0; i < LtNewestAlloc; i++)
		free(LtAllocs[LtNewestAlloc]);
}

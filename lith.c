#define _GNU_SOURCE
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define cosθ cos(θ)
#define sinθ sin(θ)

typedef float LtFloat;
typedef int   LtInt;
typedef void* LtPtr;

struct Vertex {
	LtInt a, b;
};

struct Point3i {
	LtInt x, y, z;
};

struct Point3 {
	LtFloat x, y, z;
};

struct Point2 {
	LtFloat x, y;
};

struct Point2i {
	LtInt x, y;
};

struct Buffer {
	LtInt *fb;
};

struct Mesh {
	struct Vertex *verts;
	struct Point3 *vects;
	LtInt am, ram;
};

enum Error {
	LT_ALLOC,
	LT_BOUNDS,
};

#define Vector3 Point3
#define Vector2 Point2

typedef struct Buffer  Buffer;
typedef struct Point2  LtPoint2;
typedef struct Point3  Point3;
typedef struct Point2i LtPoint2i;
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
typedef struct Point2i Ltint2i;
typedef struct Pointi  LtPointi;
typedef struct Vertex  LtVertex;
typedef struct Vector2 LtVector2;
typedef struct Vector3 LtVector3;
typedef struct Mesh    LtMesh;
typedef enum   Error   LtError;

LtInt   LtWidth       = 0;
LtInt   LtHeight      = 0;
LtFloat LtFocalLength = 0;
LtPtr   LtAllocs[65536] = { 0 };
LtInt   LtNewestAlloc = 0;

LtPoint2i ltNDCToScreen(Point2 v, LtInt w, LtInt h)
{
	LtPoint2i screenCoord;
	screenCoord.x = (LtInt) ((v.x + 1.0f) * 0.5f * w);
	screenCoord.y = (LtInt) ((1.0f - v.y) * 0.5f * h);
	screenCoord.x = screenCoord.x >= w ? w - 1 : screenCoord.x;
	screenCoord.y = screenCoord.y >= h ? h - 1 : screenCoord.y;
	screenCoord.x = screenCoord.x < 0 ? 1 : screenCoord.x;
	screenCoord.y = screenCoord.y < 0 ? 1 : screenCoord.y;
	return screenCoord;
}

LtPoint3 ltScale3DVector(LtPoint3 v, LtFloat s)
{
	return (LtPoint3) {v.x * s, v.y * s, v.z * s};
}

LtPoint3 ltAdd3DVectors(LtPoint3 v0, Point3 v1)
{
	return (LtPoint3) {v0.x + v1.x, v0.y + v1.y, v0.z + v1.z};
}

LtPoint3 ltSub3DVectors(LtPoint3 v0, Point3 v1)
{
	return (LtPoint3) {v0.x - v1.x, v0.y - v1.y, v0.z - v1.z};
}

LtPoint2i ltProject(LtPoint3 p, LtFloat foc, LtInt w, LtInt h, LtPoint3 cam)
{
	LtPoint3 view = ltSub3DVectors(p, cam);

	LtFloat pX = (view.x * foc) / fmax(view.z + foc - 0.02f, 0.01f);
	LtFloat pY = (view.y * foc) / fmax(view.z + foc - 0.02f, 0.01f);

	LtPoint2 pr = (Point2) {pX, pY};

	return ltNDCToScreen(pr, w, h);
}

void ltTransform(Mesh *m, void (*f)(LtPoint3 *))
{
	for (LtInt i = 0; i < m->am; i++)
		f(&m->vects[i]);
}

void ltClear(Buffer *buf, LtInt col)
{
	for (LtInt i = 0; i < LtWidth * LtHeight; i++)
		buf->fb[i] = col;
}

void ltDrawPixel(LtPoint2i p, LtInt col, Buffer *b)
{
	b->fb[p.x + p.y * LtWidth] = col;
}

void ltDrawLine(LtPoint2i v1, LtPoint2i v2, LtInt col, Buffer *b)
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
			ltDrawPixel((LtPoint2i) {x0, y0}, col, b);
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

void ltRenderMesh(LtInt col, Buffer *buf, Vertex *vertices, LtPoint3 *points, LtInt vert_am, LtPoint3 cam)
{
	LtInt   w   = LtWidth;
	LtInt   h   = LtHeight;
	LtFloat foc = LtFocalLength;

	for (LtInt i = 0; i < vert_am; i++) {
		LtPoint3 a = points[vertices[i].a];
		LtPoint3 b = points[vertices[i].b];

		LtPoint2i Pa = ltProject(a, foc, w, h, cam);
		LtPoint2i Pb = ltProject(b, foc, w, h, cam);

		ltDrawLine(Pa, Pb, col, buf);
	}
}

void ltPrintb(Buffer *b)
{
	LtInt w = LtWidth;
	LtInt h = LtHeight;

	for (LtInt y = 0; y < h; y++) {
		for (LtInt x = 0; x < w; x++)
			printf("%d", b->fb[y * w + x]);
		putchar('\n');
	}
}

LtPoint3 ltRotateX(Point3 in, LtFloat θ)
{
	LtFloat x´, y´, z´, x, y, z;
	x = in.x; y = in.y; z = in.z;

	x´ = x;
	y´ = y * cosθ - z * sinθ;
	z´ = y * sinθ + z * cosθ;


	return (LtPoint3) {x´, y´, z´};
}

LtPoint3 ltRotateY(Point3 in, LtFloat θ)
{
	LtFloat x´, y´, z´, x, y, z;
	x = in.x; y = in.y; z = in.z;

	x´ = x * cosθ - z * sinθ;
	y´ = y;
	z´ = x * sinθ + z * cosθ;

	return (LtPoint3) {x´, y´, z´};
}

LtPoint3 ltRotateZ(Point3 in, LtFloat θ)
{
	LtFloat x´, y´, z´, x, y, z;
	x = in.x; y = in.y; z = in.z;

	x´ = x * cosθ - y * sinθ;
	y´ = x * sinθ + y * cosθ;
	z´ = z;

	return (LtPoint3) {x´, y´, z´};
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

//void ltSaveLTObjectFile(string path, Mesh mesh)
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

	LtInt mode = 0; // 0 is invalid
		      // 1 is poLtInts/vectors
		      // 2 is vertices

	LtInt len = 0;

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
			LtFloat x, y, z;
			char *floatX = &line[0];
			char *floatY = &line[4];
			char *floatZ = &line[8];
			memcpy(&x, floatX, sizeof(LtFloat));
			memcpy(&y, floatY, sizeof(LtFloat));
			memcpy(&z, floatZ, sizeof(LtFloat));

			mesh.vects[mesh.am] = (LtPoint3) {x, y, z};
			mesh.am++;
		} else if (mode == 2) {
			LtInt a, b;
			char *intA = &line[0];
			char *intB = &line[4];
			memcpy(&a, intA, sizeof(LtInt));
			memcpy(&b, intB, sizeof(LtInt));

			mesh.verts[mesh.ram] = (Vertex) {a, b};

			mesh.ram++;
		}

	}

	printf("Loaded %d Vectors (LtPoints) and %d Vertices (Edges)\n", mesh.am, mesh.ram);

	fclose(file);
	free(line);

	return mesh;
}

Buffer ltInitBuffer(void)
{
	Buffer a;

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

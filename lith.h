#ifndef LIBLITH_HDR
#define LIBLITH_HDR 1

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

Point2i ltNDCToScreen(Point2 v, int w, int h);
Point3 ltScale3DVector(Point3 v, float s);
Point3 ltAdd3DVectors(Point3 v0, Point3 v1);
Point3 ltSub3DVectors(Point3 v0, Point3 v1);
Point2i ltProject(Point3 p, float foc, int w, int h, Point3 cam);
void ltDrawPixel(Point2i p, int col, Buffer *b, int w, int h);
void ltDrawLine(Point2i v1, Point2i v2, int col, Buffer *b, int w, int h);
void ltRenderMesh(int col, Buffer *buf, Vertex *vertices, Point3 *points, LtInt vert_am, Point3 cam);
void ltPrintb(Buffer *b);
Point3 ltRotateX(Point3 in, float θ);
Point3 ltRotateY(Point3 in, float θ);
Point3 ltRotateZ(Point3 in, float θ);
float ltDegreeToRads(float deg);
void ltPushAlloc(void *alloc);
void ltHeapDebug(void);
void ltLogError(LtError err, char *fmt, ...);
Mesh ltReadLTObjectFile(char *path);
Buffer ltInitBuffer(void);
void ltInit(LtFloat foc, LtInt w, LtInt h);
void ltTerminate(void);

#endif // LIBLITH_HDR

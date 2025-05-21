#ifndef LIBLITH_HDR
#define LIBLITH_HDR 1

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

vec2i ltNDCToScreen(vec2 v, LtInt w, LtInt h);
vec3 ltScale3Dvec(vec3 v, LtFloat s);
vec3 ltAdd3Dvecs(vec3 v0, vec3 v1);
vec3 ltSub3Dvecs(vec3 v0, vec3 v1);
vec2i ltProject(vec3 p, LtFloat foc, LtInt w, LtInt h, vec3 cam);
void ltTransform(mesh *m, void (*f)(vec3 *));
void ltClear(buffer *buf, LtInt col);
void ltDrawPixel(vec2i p, LtInt col, buffer *b);
void ltDrawLine(vec2i v1, vec2i v2, LtInt col, buffer *b);
void ltDrawTriangle(triangle tri, vec3 *points, buffer *buf, LtInt col, vec3 cam);
void ltDrawTriangles(triangle *tris, vec3 *points, buffer *buf, LtInt *cols, LtInt tricount, vec3 cam);
void ltRenderMesh(LtInt col, buffer *buf, edge *edges, vec3 *points, LtInt vert_am, vec3 cam);
void ltPrintb(buffer *b);
vec3 ltRotateX(vec3 in, LtFloat θ);
vec3 ltRotateY(vec3 in, LtFloat θ);
vec3 ltRotateZ(vec3 in, LtFloat θ);
LtFloat ltDegreeToRads(LtFloat deg);
void ltPushAlloc(void *alloc);
void ltHeapDebug(void);
void ltLogError(error err, char *fmt, ...);
mesh ltReadLTObjectFile(char *path);
buffer ltInitBuffer(void);
void ltInit(LtFloat foc, LtInt w, LtInt h);
void ltTerminate(void);

#endif // LIBLITH_HDR

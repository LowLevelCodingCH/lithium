#include <lith.h>

int main()
{
	ltInit(1.75f, 40, 40);

	LtPoint3 vectors[] = {
		{-1.0f, -1.0f, -1.0f},
		{-1.0f, -1.0f, 1.0f},
		{1.0f,  -1.0f, -1.0f},
		{-1.0f, 1.0f,  -1.0f},
		{-1.0f, 1.0f,  1.0f},
		{1.0f,  -1.0f, 1.0f},
		{1.0f,  1.0f,  -1.0f},
		{1.0f,  1.0f,  1.0f}
	};

	LtVertex vertices[] = {
		{0, 1}, {0, 2}, {0, 3},
		{2, 5}, {3, 6}, {3, 4},
		{4, 7}, {6, 7}, {7, 5},
		{5, 1}, {4, 1}, {2, 6},
	};

	LtMesh mesh;
	mesh.verts = vertices;
	mesh.vects = vectors;

	int color = 1;
	LtPoint3 cam = (Point3) {0.0f, 0.0f, -3.0f};
	float angle = 0.0f;

	LtBuffer fb = ltInitBuffer();

	ltRenderMesh(color, &fb, mesh.verts, mesh.vects, 12, cam);
	ltPrintb(&fb);

	ltHeapDebug();

	ltTerminate();
}

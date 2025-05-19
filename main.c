#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <lith.h>

float angle = 0.0f;

void fun(LtPoint3 *v)
{
	*v = ltRotateY(*v, angle);
}

int main()
{
	ltInit(1.75f, 40, 40);

	LtMesh mesh = ltReadLTObjectFile("cube.ltobj");
	LtPoint3 cam = (Point3) {0.0f, 0.0f, -3.0f};
	LtBuffer fb = ltInitBuffer();

	angle += ltDegreeToRads(1);
	ltTransform(&mesh, fun);
	ltRenderMesh(1, &fb, mesh.verts, mesh.vects, 12, cam);
	ltPrintb(&fb);
	ltClear(&fb, 0);
	usleep(100000);

	ltHeapDebug();
	ltTerminate();
}

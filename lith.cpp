#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <format>
#include <cstring>
#include <unistd.h>
using namespace std;

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
	vector<int> fb;
};

typedef Point3 Vector3;
typedef Point2 Vector2;

struct Mesh {
	vector<Vertex> verts;
	vector<Point3> vects;
	int am, ram;
};

Point2i ltNDCToScreen(Point2 v, int w, int h)
{
	Point2i screenCoord;
	screenCoord.x = static_cast<int>((v.x + 1.0f) * 0.5f * w);
	screenCoord.y = static_cast<int>((1.0f - v.y) * 0.5f * h);
	screenCoord.x = screenCoord.x >= w ? w - 1 : screenCoord.x;
	screenCoord.y = screenCoord.y >= h ? h - 1 : screenCoord.y;
	screenCoord.x = screenCoord.x < 0 ? 1 : screenCoord.x;
	screenCoord.y = screenCoord.y < 0 ? 1 : screenCoord.y;
	return screenCoord;
}

Point3 ltScale3DVector(Point3 v, float s) {
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

	float pX = (view.x * foc) / max(view.z + foc - 0.02f, 0.01f);
	float pY = (view.y * foc) / max(view.z + foc - 0.02f, 0.01f);

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

void ltRenderMesh(float foc, int w, int h, int col, Buffer *buf, vector<Vertex> &vertices, vector<Point3> &points, Point3 cam)
{
	for (auto &vertex : vertices) {
		Point3 a = points[vertex.a];
		Point3 b = points[vertex.b];

		Point2i Pa = ltProject(a, foc, w, h, cam);
		Point2i Pb = ltProject(b, foc, w, h, cam);

		ltDrawLine(Pa, Pb, col, buf, w, h);
	}
}

void ltPrintb(int w, int h, Buffer *b)
{
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++)
			cout << b->fb[y * w + x];
		cout << endl;
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

void ltSaveLTObjectFile(string path, Mesh mesh)
{
	ofstream file(path, ios::binary);

	file << "LTObjF\n";
	file << "vects\n";
	
	for (auto &vec : mesh.vects) {
		vector<char> th(3 * sizeof(float), 0);

		memcpy(&th[0], &vec.x, sizeof(float));
		memcpy(&th[4], &vec.y, sizeof(float));
		memcpy(&th[8], &vec.z, sizeof(float));

		file.write(reinterpret_cast<char*> (th.data()), th.size());
		file << "\n";
	}

	file << "verts\n";
	
	for (auto &ver : mesh.verts) {
		vector<char> th(2 * sizeof(int), 0);

		memcpy(&th[0], &ver.a, sizeof(int));
		memcpy(&th[4], &ver.b, sizeof(int));

		file.write(reinterpret_cast<char*> (th.data()), th.size());
		file << "\n";
	}
}

Mesh ltReadLTObjectFile(string path)
{
	Mesh mesh;
	mesh.am = 0;
	mesh.ram = 0;
	ifstream file;
	file.open(path);

	string line;

	int mode = 0; // 0 is invalid
		      // 1 is points/vectors
		      // 2 is vertices

	getline(file, line);
	if (line != "LTObjF") return mesh;

	while (getline(file, line)) {
		if (line == "") continue;

		if (line == "vects") {
			mode = 1;
			continue;
		} else if (line == "verts") {
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

			mesh.vects.push_back((Point3) {x, y, z});
			mesh.am++;
		} else if (mode == 2) {
			int a, b;
			char *intA = &line[0];
			char *intB = &line[4];
			memcpy(&a, intA, sizeof(int));
			memcpy(&b, intB, sizeof(int));

			mesh.verts.push_back((Vertex) {a, b});

			mesh.ram++;
		}
	}

	cout << format("Loaded {} Vectors (Points) and {} Vertices (Edges)\n", mesh.am, mesh.ram);

	file.close();

	return mesh;
}

int main()
{
	vector<Point3> basePoints = {
		{-1.0f, -1.0f, -1.0f},
		{-1.0f, -1.0f, 1.0f},
		{1.0f,  -1.0f, -1.0f},
		{-1.0f, 1.0f,  -1.0f},
		{-1.0f, 1.0f,  1.0f},
		{1.0f,  -1.0f, 1.0f},
		{1.0f,  1.0f,  -1.0f},
		{1.0f,  1.0f,  1.0f}
	};

	vector<Vertex> vertices = {
		{0, 1}, {0, 2}, {0, 3},
		{2, 5}, {3, 6}, {3, 4},
		{4, 7}, {6, 7}, {7, 5},
		{5, 1}, {4, 1}, {2, 6},
	};

	Mesh mesh;
	//mesh.verts = vertices;
	//mesh.vects = basePoints;

	mesh = ltReadLTObjectFile("cube.ltobj");

	//ltSaveLTObjectFile("cube.ltobj", mesh);

	int color = 1;
	int w = 40,
	    h = 40;
	Point3 cam = (Point3) {0.0f, 0.0f, -3.0f};
	float foc_len = 1.753f;
	float angle = 0.0f;

	while (true) {
		Buffer fb;
		fb.fb.resize(w * h);
		vector<Point3> points = mesh.vects;

		for (int i = 0; i < mesh.am; i++) {
			points[i] = ltRotateY(points[i], angle);
		}

		ltRenderMesh(foc_len, w, h, color, &fb, mesh.verts, points, cam);
		ltPrintb(w, h, &fb);

		usleep(10000);
		cout << "\033[2J\033[1;1H";

		angle += ltDegreeToRads(1.25f);
	}
}

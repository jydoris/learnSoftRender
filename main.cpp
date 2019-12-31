#include <vector>
#include "tgaimage.h"
#include "model.h"
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
const int width = 800;
const int height = 800;

struct location
{
	int m_x;
	int m_y;
	location() {}
	location(int x, int y) { m_x = x; m_y = y; }
};

void line(location p0, location p1, TGAImage &image, TGAColor color) {
	
	bool isSteep = false;

	//transpose the line by axis y = x
	if (std::abs(p1.m_y - p0.m_y) > std::abs(p1.m_x - p0.m_x)) {
		std::swap(p0.m_x, p0.m_y);
		std::swap(p1.m_x, p1.m_y);
		isSteep = true;
	}

	if (p0.m_x > p1.m_x) {
		std::swap(p0.m_x, p1.m_x);
		std::swap(p0.m_y, p1.m_y);
	}

	int dy = p1.m_y - p0.m_y;
	int dx = p1.m_x - p0.m_x;
	int unitIncre = 2 * std::abs(dy);
	float accuIncre = 0;
	for (int x = p0.m_x, y = p0.m_y; x <= p1.m_x; x++) {
		if (isSteep)
			image.set(y, x, color);
		else
			image.set(x, y, color);
		
		accuIncre += unitIncre;
		if (accuIncre > dx) { //need to increase or decrease y now, rather than stay at y
			y += (dy > 0 ? 1 : -1);
			accuIncre -= 2.0 * dx;   //adjust the accumulate increase, its always based on the new height
		}
	}
	
}

void triangle(location p0, location p1, location p2, TGAImage &image, TGAColor color) {

	if (p0.m_y == p1.m_y && p1.m_y == p2.m_y) {
		return;
	}

	//y increase by p0, p1, p2
	if(p0.m_y > p1.m_y) {
		std::swap(p0, p1);
	}
	if(p0.m_y > p2.m_y){
		std::swap(p0, p2);
	}
	if (p1.m_y > p2.m_y) {
		std::swap(p1, p2);
	}

	float total_height = p2.m_y - p0.m_y;

	for (int y = 0; y <= total_height; y++) {
		bool secondPart = (y + p0.m_y >= p1.m_y) ? true : false;
		float segment_height = secondPart ? p2.m_y - p1.m_y + 0.01 : p1.m_y - p0.m_y + 0.01; //avoid zero
		int start = secondPart ? p1.m_x + (y + p0.m_y - p1.m_y) / segment_height * (p2.m_x - p1.m_x) : p0.m_x + y / segment_height * (p1.m_x - p0.m_x);
		int end = p0.m_x + y / total_height * (p2.m_x - p0.m_x);
		if (start > end) {
			std::swap(start, end);
		}
		for (int j = start; j <= end; j++) {
			image.set(j, y + p0.m_y, color);
		}
		
	}
}

void rasterization(location p0, location p1, TGAImage &image, int yBuffer[], TGAColor color) {
	if (p0.m_x > p1.m_x) {
		std::swap(p0, p1);
	}

	for (int x = p0.m_x; x <= p1.m_x; x++) {
		float t = (x - p0.m_x) / (float)(p1.m_x - p0.m_x);
		int y = p0.m_y * (1.0 - t) + p1.m_y * t;
		if (y > yBuffer[x]) {
			yBuffer[x] = y;
			for (int j = 0; j < 20; j++)
			{
				image.set(x, j, color);
			}
		}
	}
}

int main(int argc, char** argv) {
	TGAImage scene(width, 50, TGAImage::RGB);


	/*Model *model = new Model("obj/african_head.obj");


	Vec3f light_dir = Vec3f(0, 0, 1);
	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);

		Vec3f world_coord[3];
		location screen_coord[3];
		for (int j = 0; j < 3; j++) {
			world_coord[j] = model->vert(face[j]);
			screen_coord[j] = location((world_coord[j].x + 1.)*width / 2., (world_coord[j].y + 1.)*width / 2.);
		}

		Vec3f norm = (world_coord[0] - world_coord[1]) ^ (world_coord[0] - world_coord[2]);
		norm.normalize();
		float tensity = norm * light_dir;

		if (tensity > 0){
			triangle(screen_coord[0], screen_coord[1], screen_coord[2], image, TGAColor(tensity * 255, tensity * 255, tensity * 255, 255));
		}
	}*/

	int ybuffer[width];
	for (int i = 0; i < width; i++) {
		ybuffer[i] = std::numeric_limits<int>::min();
	}
	// scene "2d mesh"
	rasterization(location(20, 34), location(744, 400), scene, ybuffer, red);
	rasterization(location(120, 434), location(444, 400), scene, ybuffer, green);
	rasterization(location(330, 463), location(594, 200), scene, ybuffer, blue);

	scene.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	scene.write_tga_file("output.tga");
		return 0;
}
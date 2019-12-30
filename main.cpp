#include <vector>
#include "tgaimage.h"
#include "model.h"
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const int width = 200;
const int height = 200;

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

int main(int argc, char** argv) {
	TGAImage image(width, height, TGAImage::RGB);


	/*Model *model = new Model("obj/african_head.obj");


	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);

		Vec3f v0 = model->vert(face[0]);
		Vec3f v1 = model->vert(face[1]);
		Vec3f v2 = model->vert(face[2]);
		triangle(location((v0.x + 1.)*width / 2., (v0.y + 1.)*width / 2.),
			location((v1.x + 1.)*width / 2., (v1.y + 1.)*width / 2.),
			location((v2.x + 1.)*width / 2., (v2.y + 1.)*width / 2.),
			image, green);
	}*/

	location p0[3] = { location(0, 0),   location(1, 100),  location(100, 100) };
	location p1[3] = { location(100, 100),  location(100, 199),   location(199, 199) };
	location p2[3] = { location(100, 100), location(199, 100), location(175, 75) };
	triangle(p0[0], p0[1], p0[2], image, white);
	triangle(p1[0], p1[1], p1[2], image, green);
	triangle(p2[0], p2[1], p2[2], image, red);


	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga"); 
		return 0;
}
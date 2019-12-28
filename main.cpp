#include <vector>
#include "tgaimage.h"
#include "model.h"
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
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

int main(int argc, char** argv) {
	TGAImage image(width, height, TGAImage::RGB);


	Model *model = new Model("obj/african_head.obj");


	for (int i = 0; i<model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		for (int j = 0; j<3; j++) {
			Vec3f v0 = model->vert(face[j]);
			Vec3f v1 = model->vert(face[(j + 1) % 3]);
			int x0 = (v0.x + 1.)*width / 2.;
			int y0 = (v0.y + 1.)*height / 2.;
			int x1 = (v1.x + 1.)*width / 2.;
			int y1 = (v1.y + 1.)*height / 2.;
			line(location(x0, y0), location(x1, y1), image, white);
		}
	}


	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga"); 
		return 0;
}
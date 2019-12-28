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

void triangle(location p0, location p1, location p2, TGAImage &image, TGAColor color) {
	line(p0, p1, image, color);
	line(p1, p2, image, color);
	line(p2, p0, image, color);
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

	location t0[3] = { location(10, 70),   location(50, 160),  location(70, 80) };
	location t1[3] = { location(180, 50),  location(150, 1),   location(70, 180) };
	location t2[3] = { location(180, 150), location(120, 160), location(130, 180) };
	triangle(t0[0], t0[1], t0[2], image, white);
	triangle(t1[0], t1[1], t1[2], image, green);
	triangle(t2[0], t2[1], t2[2], image, red);


	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga"); 
		return 0;
}
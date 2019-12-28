#include <vector>
#include "tgaimage.h"
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);

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

std::vector<location> vertex  = {
	location(1, 1),
	location(50, 99),
	location(99, 30),
	location(1, 1),
	location(1, 30),
	location(30, 1)
};

int main(int argc, char** argv) {
	TGAImage image(100, 100, TGAImage::RGB);
	
	for(int i = 0; i < vertex.size(); i+=2)
		line(vertex[i], vertex[i+1], image, green);


	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga"); 
		return 0;
}
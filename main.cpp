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

	if (p0.m_y > p1.m_y) {
		std::swap(p0.m_x, p1.m_x);
		std::swap(p0.m_y, p1.m_y);
	}
	
	for (int x = p0.m_x; x <= p1.m_x; x++) {
		float t = (float)(x - p0.m_x) / (p1.m_x - p0.m_x);
		int y = p0.m_y * (1.0 - t) + p1.m_y * t;
		if (isSteep)
			image.set(y, x, color);
		else
			image.set(x, y, color);
	}

	
}

int main(int argc, char** argv) {
	TGAImage image(100, 100, TGAImage::RGB);
	location p0(1, 1);
	location p1(20, 50);
	line(p1, p0, image, green);
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga"); 
		return 0;
}
#include <vector>
#include "tgaimage.h"
#include "model.h"
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
const int width = 800;
const int height = 800;
float zbuffer[width][height];

struct location
{
	int m_x;
	int m_y;
	location() {}
	location(int x, int y) { m_x = x; m_y = y; }
};

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
	Vec3f s[2];
	for (int i = 2; i--; ) {
		s[i][0] = C[i] - A[i];
		s[i][1] = B[i] - A[i];
		s[i][2] = A[i] - P[i];
	}
	Vec3f u = cross(s[0], s[1]);
	if (std::abs(u[2])>1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
		return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	return Vec3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

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

//no z buffer
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

void rasterization(Vec3f p0, Vec3f p1, Vec3f p2, TGAImage &image, float zBuffer[][height], TGAColor color) {
	if (p0.y == p1.y && p1.y == p2.y) {
		return;
	}

	//y increase by p0, p1, p2
	if (p0.y > p1.y) {
		std::swap(p0, p1);
	}
	if (p0.y > p2.y) {
		std::swap(p0, p2);
	}
	if (p1.y > p2.y) {
		std::swap(p1, p2);
	}

	float total_height = p2.y - p0.y;

	int screenPosX;
	int screenPosY;

	for (int y = 0; y <= total_height; y++) {
		bool secondPart = (y + p0.y >= p1.y) ? true : false;
		float segment_height = secondPart ? p2.y - p1.y + 0.01 : p1.y - p0.y + 0.01; //avoid zero
		int start = secondPart ? p1.x + (y + p0.y - p1.y) / segment_height * (p2.x - p1.x) : p0.x + y / segment_height * (p1.x - p0.x);
		int end = p0.x + y / total_height * (p2.x - p0.x);
		if (start > end) {
			std::swap(start, end);
		}

		for (int j = start; j <= end; j++) {
			screenPosX = j;
			screenPosY = y + p0.y;

			Vec3f factor = barycentric(p0, p1, p2, Vec3f(j, y + p0.y, 0)); //value of z axis won't be used
			if (factor.x < 0 || factor.y < 0 || factor.z < 0) continue;
			float z = factor[0] * p0.z + factor[1] * p1.z + factor[2] * p2.z;

			if (z > zBuffer[screenPosX][screenPosY]) {
				image.set(screenPosX, screenPosY, color);
				zBuffer[screenPosX][screenPosY] = z;
			}
		}

	}
}


Vec3f tex2screen(int twidth, int theight, Vec3f v) {
	if(v.x < 0.0) v.x = 0.0;
	if (v.x > 1.0) v.x = 1.0;
	if (v.y < 0.0) v.y = 0.0;
	if (v.y > 1.0) v.y = 1.0;
	return Vec3f(v.x*twidth, v.y*theight, v.z);
}

void rasterization(Vec3f p0, Vec3f p1, Vec3f p2, TGAImage &image, float zBuffer[][height], float intensity, TGAImage &textureImage, Vec3f tex_coord[]) {
	if (p0.y == p1.y && p1.y == p2.y) {
		return;
	}

	//y increase by p0, p1, p2
	if (p0.y > p1.y) {
		std::swap(p0, p1);
		std::swap(tex_coord[0], tex_coord[1]); //remember to do this, or the color will be not be smooth
	}
	if (p0.y > p2.y) {
		std::swap(p0, p2);
		std::swap(tex_coord[0], tex_coord[2]);
	}
	if (p1.y > p2.y) {
		std::swap(p1, p2);
		std::swap(tex_coord[1], tex_coord[2]);
	}

	float total_height = p2.y - p0.y;

	int screenPosX;
	int screenPosY;

	for (int y = 0; y <= total_height; y++) {
		bool secondPart = (y + p0.y >= p1.y) ? true : false;
		float segment_height = secondPart ? p2.y - p1.y + 0.01 : p1.y - p0.y + 0.01; //avoid zero
		int start = secondPart ? p1.x + (y + p0.y - p1.y) / segment_height * (p2.x - p1.x) : p0.x + y / segment_height * (p1.x - p0.x);
		int end = p0.x + y / total_height * (p2.x - p0.x);
		if (start > end) {
			std::swap(start, end);
		}

		for (int j = start; j <= end; j++) {
			screenPosX = j;
			screenPosY = y + p0.y;

			Vec3f factor = barycentric(p0, p1, p2, Vec3f(screenPosX, screenPosY, 0)); //value of z axis won't be used
			if (factor.x < 0 || factor.y < 0 || factor.z < 0) continue;
			float z = factor[0] * p0.z + factor[1] * p1.z + factor[2] * p2.z;

			Vec3f interTex;
			for (int i = 0; i < 3; i++) {
				interTex[i] = factor[0] * tex_coord[0][i] + factor[1] * tex_coord[1][i] + factor[2] * tex_coord[2][i];
		
			}

			Vec3f texScreenCoord = tex2screen(textureImage.get_width(), textureImage.get_height(), interTex);

			TGAColor color = textureImage.get(texScreenCoord.x, texScreenCoord.y);
			if (z > zBuffer[screenPosX][screenPosY]) {
				image.set(screenPosX, screenPosY, TGAColor(intensity*color[0], intensity*color[1], intensity*color[2]));
				zBuffer[screenPosX][screenPosY] = z;
			}
		}

	}
}

Vec3f world2screen(Vec3f v) {
	return Vec3f(int((v.x + 1.)*width / 2. + .5), int((v.y + 1.)*height / 2. + .5), v.z);
}



int main(int argc, char** argv) {
	TGAImage scene(width, height, TGAImage::RGB);
	TGAImage textureImage;
	textureImage.read_tga_file("obj/african_head_diffuse.tga");
	textureImage.flip_vertically();

	
	for (int i = 0; i < width; i++) {
		for(int j = 0; j < height; j++)
			zbuffer[i][j] = -std::numeric_limits<float>::max();
	}

	Model *model = new Model("obj/african_head.obj");


	Vec3f light_dir = Vec3f(0, 0, 1);
	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		std::vector<int> texts = model->texures(i);

		Vec3f world_coord[3];
		Vec3f tex_coord[3];
		for (int j = 0; j < 3; j++) {
			world_coord[j] = model->vert(face[j]);
			tex_coord[j] = model->text(texts[j]);
		}

		Vec3f norm = cross((world_coord[0] - world_coord[1]), (world_coord[0] - world_coord[2]));
		norm.normalize();
		float tensity = norm * light_dir;
		
		
		if (tensity > 0){
			rasterization(world2screen(world_coord[0]), world2screen(world_coord[1]), world2screen(world_coord[2]), scene, zbuffer,tensity, textureImage, tex_coord);
		}
	}


	scene.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	scene.write_tga_file("output.tga");

	/*std::cout << "yes" << std::endl;
	system("pause");*/
		return 0;
}
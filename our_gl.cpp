#include "our_gl.h"

Matrix ModelView;
Matrix Viewport;
Matrix Projection;
extern Vec3f light_dir;

void lookat(Vec3f eye, Vec3f center, Vec3f up) 
{
	Vec3f z = (eye - center).normalize();
	Vec3f x = cross(up, z).normalize();
	Vec3f y = cross(z, x).normalize();
	Matrix Minv = Matrix::identity();
	Matrix Tr = Matrix::identity();
	for (int i = 0; i<3; i++) {
		Minv[0][i] = x[i];
		Minv[1][i] = y[i];
		Minv[2][i] = z[i];
		Tr[i][3] = -center[i];
	}
	ModelView = Minv * Tr;
}

void viewport(int x, int y, int w, int h) 
{
	Viewport = Matrix::identity();
	Viewport[0][3] = x + w / 2.f;
	Viewport[1][3] = y + h / 2.f;
	Viewport[2][3] = 255 / 2.0; // 1.0 //why change from depth / 2.0 ---> 1.0

	Viewport[0][0] = w / 2.f;
	Viewport[1][1] = h / 2.f;
	Viewport[2][2] = 255 / 2.0; // 0.0; //
}


void projection(Vec3f cameraPos, Vec3f center)
{
	Projection = Matrix::identity();
	Projection[3][2] = -1.0 / (cameraPos - center).norm();
}




Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P)
{
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


void line(Vec2f p0, Vec2f p1, TGAImage &image, TGAColor color)
{

	bool isSteep = false;

	//transpose the line by axis y = x
	if (std::abs(p1.y - p0.y) > std::abs(p1.x - p0.x)) {
		std::swap(p0.x, p0.y);
		std::swap(p1.x, p1.y);
		isSteep = true;
	}

	if (p0.x > p1.x) {
		std::swap(p0.x, p1.x);
		std::swap(p0.y, p1.y);
	}

	int dy = p1.y - p0.y;
	int dx = p1.x - p0.x;
	int unitIncre = 2 * std::abs(dy);
	float accuIncre = 0;
	for (int x = p0.x, y = p0.y; x <= p1.x; x++) {
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
void triangle(Vec2f p0, Vec2f p1, Vec2f p2, TGAImage &image, TGAColor color) 
{

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

	for (int y = 0; y <= total_height; y++) {
		bool secondPart = (y + p0.y >= p1.y) ? true : false;
		float segment_height = secondPart ? p2.y - p1.y + 0.01 : p1.y - p0.y + 0.01; //avoid zero
		int start = secondPart ? p1.x + (y + p0.y - p1.y) / segment_height * (p2.x - p1.x) : p0.x + y / segment_height * (p1.x - p0.x);
		int end = p0.x + y / total_height * (p2.x - p0.x);
		if (start > end) {
			std::swap(start, end);
		}
		for (int j = start; j <= end; j++) {
			image.set(j, y + p0.y, color);
		}

	}
}

//without texture
void rasterization(Vec3f p0, Vec3f p1, Vec3f p2, TGAImage &image, float *zBuffer, TGAColor color) 
{
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
	int image_width = image.get_width();

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

			if (z > zBuffer[screenPosX + image_width * screenPosY]) {
				image.set(screenPosX, screenPosY, color);
				zBuffer[screenPosX + image_width * screenPosY] = z;
			}
		}

	}
}

Vec3f tex2screen(int twidth, int theight, Vec3f v) {
	if (v.x < 0.0) v.x = 0.0;
	if (v.x > 1.0) v.x = 1.0;
	if (v.y < 0.0) v.y = 0.0;
	if (v.y > 1.0) v.y = 1.0;
	return Vec3f(v.x*twidth, v.y*theight, v.z);
}

void rasterization(Vec3f p0, Vec3f p1, Vec3f p2, TGAImage &image, float *zBuffer, Vec3f norm_coord[], TGAImage &textureImage, Vec3f tex_coord[]) {
	if (p0.y == p1.y && p1.y == p2.y) {
		return;
	}

	//y increase by p0, p1, p2
	if (p0.y > p1.y) {
		std::swap(p0, p1);
		std::swap(tex_coord[0], tex_coord[1]); //remember to do this, or the color will be not be smooth
		std::swap(norm_coord[0], norm_coord[1]);
	}
	if (p0.y > p2.y) {
		std::swap(p0, p2);
		std::swap(tex_coord[0], tex_coord[2]);
		std::swap(norm_coord[0], norm_coord[2]);
	}
	if (p1.y > p2.y) {
		std::swap(p1, p2);
		std::swap(tex_coord[1], tex_coord[2]);
		std::swap(norm_coord[1], norm_coord[2]);
	}

	float total_height = p2.y - p0.y;

	int screenPosX;
	int screenPosY;
	int image_width = image.get_width();

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
			Vec3f interNorm;
			for (int i = 0; i < 3; i++) {
				interTex[i] = factor[0] * tex_coord[0][i] + factor[1] * tex_coord[1][i] + factor[2] * tex_coord[2][i];
				interNorm[i] = factor[0] * norm_coord[0][i] + factor[1] * norm_coord[1][i] + factor[2] * norm_coord[2][i];
			}


			//texture color
			Vec3f texScreenCoord = tex2screen(textureImage.get_width(), textureImage.get_height(), interTex);

			TGAColor color = textureImage.get(texScreenCoord.x, texScreenCoord.y);

			//intensit using interpolate norm
			interNorm.normalize();
			float intensity = interNorm * light_dir;
			if (z > zBuffer[screenPosX + image_width * screenPosY] && intensity > 0) {
				image.set(screenPosX, screenPosY, TGAColor(intensity*color[0], intensity*color[1], intensity*color[2]));

				zBuffer[screenPosX + image_width * screenPosY] = z;
			}
		}

	}
}







Vec3f homo2Vec3(Vec4f h)
{
	Vec3f v;
	v[0] = h[0] / h[3];
	v[1] = h[1] / h[3];
	v[2] = h[2] / h[3];
	return v;
}

Vec4f homoVec(Vec3f v) 
{
	Vec4f h;
	h[0] = v[0];
	h[1] = v[1];
	h[2] = v[2];
	h[3] = 1;
	return h;
}
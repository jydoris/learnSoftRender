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

int depth = 255;
void viewport(int x, int y, int w, int h) 
{
	Viewport = Matrix::identity();
	Viewport[0][3] = x + w / 2.f;
	Viewport[1][3] = y + h / 2.f;
    Viewport[2][3] =  depth / 2.0;

	Viewport[0][0] = w / 2.f;
	Viewport[1][1] = h / 2.f;
    Viewport[2][2] =  depth / 2.0;
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

void rasterization(TGAImage &image, float *zBuffer, Ishader &shader) {

	Vec3f p[3];
	for (int i = 0; i < 3; i++) {
		p[i] = shader.varing_pos.col(i);
		if (p[i].x < 0 || p[i].x >= image.get_width() || p[i].y < 0 || p[i].x >= image.get_height())
			return;
	}
	if (p[0].y == p[1].y && p[1].y == p[2].y) {
		return;
	}

	float total_height = p[2].y - p[0].y;

	int screenPosX;
	int screenPosY;
	int image_width = image.get_width();

	for (int y = 0; y <= total_height; y++) {
		bool secondPart = (y + p[0].y >= p[1].y) ? true : false;
		float segment_height = secondPart ? p[2].y - p[1].y + 0.01 : p[1].y - p[0].y + 0.01; //avoid zero
		int start = secondPart ? p[1].x + (y + p[0].y - p[1].y) / segment_height * (p[2].x - p[1].x) : p[0].x + y / segment_height * (p[1].x - p[0].x);
		int end = p[0].x + y / total_height * (p[2].x - p[0].x);
		if (start > end) {
			std::swap(start, end);
		}

		for (int j = start; j <= end; j++) {
			screenPosX = j;
			screenPosY = y + p[0].y;

			Vec3f factor = barycentric(p[0], p[1], p[2], Vec3f(screenPosX, screenPosY, 0)); //value of z axis won't be used
			if (factor.x < 0 || factor.y < 0 || factor.z < 0) continue;
			float z = factor[0] * p[0].z + factor[1] * p[1].z + factor[2] * p[2].z;
			
			TGAColor color;
	
			bool discard = shader.fragment(factor, color);
			if(screenPosX < 0 || screenPosX >= image_width || screenPosY <0 || screenPosY >= image.get_height())
                discard = true;
			if (!discard && z > zBuffer[screenPosX + image_width * screenPosY]) {
				image.set(screenPosX, screenPosY, TGAColor(color.r, color.g, color.b));

				zBuffer[screenPosX + image_width * screenPosY] = z;
			}
		}

	}
}




Vec3f homo2Vec3(Vec4f h)
{
    //这里要注意整形转换，否则会有部分像素缺失
	Vec3f v;
	v[0] = int(h[0] / h[3]);
	v[1] = int(h[1] / h[3]);
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

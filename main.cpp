#include <vector>
#include <algorithm>
#include "tgaimage.h"
#include "model.h"
#include "our_gl.h"
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
const int width = 800;
const int height = 800;
float *zbuffer;

Model * model;
Vec3f light_dir = Vec3f(0, 0, 1);
Vec3f center = Vec3f(0, 0, 0);
Vec3f cameraPos = Vec3f(4, 0, 5);
Vec3f up = Vec3f(0, 1, 0);

class GouraudShader : public Ishader {
public:
	mat<2, 3, float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
	mat<3, 3, float> varying_nrm; // normal per vertex to be interpolated by FS

	mat<4, 4, float> uniform_M;   //  Projection*ModelView
	mat<4, 4, float> uniform_MIT; // (Projection*ModelView).invert_transpose()


public:
	GouraudShader() {};
	void vertex(int iFace, int nthVert)
	{
		std::vector<Vec3i> face = model->face(iFace);
		
		Vec3f world_coord = model->vert(face[nthVert][0]);
		varying_uv.set_col(nthVert, model->text(face[nthVert][1]));
		varying_nrm.set_col(nthVert, model->norm(face[nthVert][2]));
		
		varing_pos.set_col(nthVert, homo2Vec3(Viewport * Projection * ModelView * homoVec(world_coord)));
	}
	bool fragment(Vec3f factor, TGAColor &desColor)
	{
		Vec2f interTex;
		Vec3f interNorm;
		
		interTex = varying_uv * factor;
		interNorm = varying_nrm * factor;
		Vec3f n = homo2Vec3(uniform_MIT*homoVec(model->normal(interTex))).normalize();
		Vec3f l = homo2Vec3(uniform_M  * homoVec(light_dir)).normalize();
		
		//intensity using interpolate norm
		interNorm.normalize();
		Vec3f r = (n*(n*l*2.f) - l).normalize();   // reflected light
		float spec = pow(std::max(r.z, 0.0f), model->specular(interTex));
		float diff = std::max(0.f, n*l);
		TGAColor c = model->diffuse(interTex);
		/*float intensity = n * l;
		if (intensity < 0) return true;
		desColor = model->diffuse(interTex) * intensity;*/
		for (int i = 0; i<3; i++) 
			desColor.raw[i] = std::min<float>(5 + c.raw[i] * (diff + 0.6*spec), 255);
		
		return false;
	}

	void sort() {
		//y increase by 0,1,2(col index)
		if (varing_pos[1][0] > varing_pos[1][1]) {
			varing_pos.swap_col(0, 1);
			varying_uv.swap_col(0, 1);
			varying_nrm.swap_col(0, 1);
		}
		if (varing_pos[1][0] > varing_pos[1][2]) {
			varing_pos.swap_col(0, 2);
			varying_uv.swap_col(0, 2);
			varying_nrm.swap_col(0, 2);
		}
		if (varing_pos[1][1] > varing_pos[1][2]) {
			varing_pos.swap_col(1, 2);
			varying_uv.swap_col(1, 2);
			varying_nrm.swap_col(1, 2);
		}
	}
};

int main(int argc, char** argv) {
	TGAImage scene(width, height, TGAImage::RGB);
	TGAImage textureImage;
	textureImage.read_tga_file("obj/african_head_diffuse.tga");
	textureImage.flip_vertically();

	zbuffer = new float[width*height];
	for (int i = 0; i < width*height; i++) {
			zbuffer[i] = -std::numeric_limits<float>::max();
	}

	lookat(cameraPos, center, up);
	viewport(0, 0, width, height);
	projection(cameraPos, center);

	model = new Model("obj/african_head.obj");

	model->loadTexture("obj/african_head_diffuse.tga");
	model->loadNoraml("obj/african_head_nm.tga");
	model->loadSpecular("obj/african_head_spec.tga");

	GouraudShader shader;
	shader.uniform_M = Projection * ModelView;
	shader.uniform_MIT = (Projection*ModelView).invert_transpose();
	for (int i = 0; i < model->nfaces(); i++) {
		for (int j = 0; j < 3; j++) {
			/**/
			shader.vertex(i, j);
		}
		shader.sort();
		rasterization(scene, zbuffer, shader);
		
	}
	delete model;

	scene.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	scene.write_tga_file("output.tga");
	delete[] zbuffer;
	/*std::cout << "yes" << std::endl;
	system("pause");*/
		return 0;
}

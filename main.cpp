#include <vector>
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

Vec3f light_dir = Vec3f(0, 0, 1);
Vec3f center = Vec3f(0, 0, 0);
Vec3f cameraPos = Vec3f(0, 0, 15);
Vec3f up = Vec3f(0, 1, 0);

class GouraudShader : public Ishader {
	//float varing_intensity;
	
	mat<3, 3, float> varing_pos;

	mat<2, 3, float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
	mat<3, 3, float> varying_nrm; // normal per vertex to be interpolated by FS
public:
	GouraudShader() {};
	void vertex(int iFace, int nthVert)
	{

		/*Vec3f world_coord[3];
		Vec3f tex_coord[3];
		Vec3f norm_coord[3];
		for (int j = 0; j < 3; j++) {
			world_coord[j] = model->vert(face[j][0]);
			tex_coord[j] = model->text(face[j][1]);
			norm_coord[j] = model->norm(face[j][2]);
		}

		lookat(cameraPos, center, up);
		viewport(0, 0, width, height);
		projection(cameraPos, center);

		Vec3f screen_coord[3];
		for (int j = 0; j < 3; j++) {
			screen_coord[j] = homo2Vec3(Viewport * Projection * ModelView * homoVec(world_coord[j]));
		}*/

	}
	void fragment(Vec3f factor, TGAColor desColor)
	{
		/*Vec3f interTex;
		Vec3f interNorm;
		for (int i = 0; i < 3; i++) {
			interTex[i] = factor[0] * tex_coord[0][i] + factor[1] * tex_coord[1][i] + factor[2] * tex_coord[2][i];
			interNorm[i] = factor[0] * norm_coord[0][i] + factor[1] * norm_coord[1][i] + factor[2] * norm_coord[2][i];
		}*/

		//intensity using interpolate norm
		/*interNorm.normalize();
		float intensity = interNorm * light_dir;*/
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

	Model *model = new Model("obj/african_head.obj");

	model->loadTexture("obj/african_head_diffuse.tga");

	GouraudShader shader;
	for (int i = 0; i < model->nfaces(); i++) {
		for (int j = 0; j < 3; j++) {
			/*std::vector<Vec3i> face = model->face(i);*/
			shader.vertex(i, j);
		}
		
		rasterization(scene, zbuffer, shader);
		
	}


	scene.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	scene.write_tga_file("output.tga");
	delete[] zbuffer;
	/*std::cout << "yes" << std::endl;
	system("pause");*/
		return 0;
}

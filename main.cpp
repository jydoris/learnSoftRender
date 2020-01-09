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

int main(int argc, char** argv) {
	TGAImage scene(width, height, TGAImage::RGB);
	TGAImage textureImage;
	textureImage.read_tga_file("/Users/doris/Desktop/GIT/learnSoftRender/obj/african_head_diffuse.tga");
	textureImage.flip_vertically();

	zbuffer = new float[width*height];
	for (int i = 0; i < width*height; i++) {
			zbuffer[i] = -std::numeric_limits<float>::max();
	}

	Model *model = new Model("/Users/doris/Desktop/GIT/learnSoftRender/obj/african_head.obj");


	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<Vec3i> face = model->face(i);

		Vec3f world_coord[3];
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
		}

        rasterization(screen_coord[0], screen_coord[1], screen_coord[2], scene, zbuffer,norm_coord, textureImage, tex_coord);
	}


	scene.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	scene.write_tga_file("output.tga");
	delete[] zbuffer;
	/*std::cout << "yes" << std::endl;
	system("pause");*/
		return 0;
}

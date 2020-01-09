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
Vec3f cameraPos = Vec3f(0, -3, 15);
Vec3f up = Vec3f(0, 1, 0);

Vec3f world2screen(Vec3f v) {
    return Vec3f(int((v.x + 1.)*width / 2.), int((v.y + 1.)*height / 2.), v.z);
}
Vec4f world2screen(Vec4f v) {
    Vec4f res;
    res[0] = (v[0] + v[3])*width / 2.;
    res[1] = (v[1] + v[3])*height / 2.;
    res[2] = v[2];
    res[3] = v[3];

    return res;
}

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
//            auto a = (Projection * ModelView * homoVec(world_coord[j]));
//            Vec4f res = homo2Vec4(a);
//            screen_coord[j] = homo2Vec3(Viewport * res);
            screen_coord[j] = homo2Vec3(Viewport * Projection * ModelView * homoVec(world_coord[j]));

            //先转换齐次坐标，再视口转换
//            screen_coord[j] = homo2Vec3(a);
//            screen_coord[j] = world2screen(screen_coord[j]);

//          //先视口变换，再其次坐标转化
            //注意这里的转换坐标不一样了，而且要注意整形转换
//            auto mid = world2screen(a);
//            screen_coord[j] = homo2Vec3(mid);
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

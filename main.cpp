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
float *shadowBuffer;

Model * model;
Vec3f light_dir = Vec3f(0, 5, 5);
Vec3f center = Vec3f(0, 0, 0);
Vec3f cameraPos = Vec3f(1, 1, 5);
Vec3f up = Vec3f(0, 1, 0);

class Shader : public Ishader {
public:
    mat<2, 3, float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
    mat<3, 3, float> varying_nrm; // normal per vertex to be interpolated by FS

    mat<4, 4, float> uniform_M;   //  Projection*ModelView
    mat<4, 4, float> uniform_MIT; // (Projection*ModelView).invert_transpose()
	mat<4, 4, float> uniform_ShadowTran;


public:
    Shader() {};
	Shader(Matrix M, Matrix MIT, Matrix shadowTrans) {
		uniform_M = M;
		uniform_MIT = MIT;
		uniform_ShadowTran = shadowTrans;
	}
    void vertex(int iFace, int nthVert)
    {
        std::vector<Vec3i> face = model->face(iFace);

        Vec3f world_coord = model->vert(face[nthVert][0]);
        varying_uv.set_col(nthVert, model->text(face[nthVert][1]));
        varying_nrm.set_col(nthVert, homo2Vec3(uniform_MIT * homoVec(model->norm(face[nthVert][2]))));

        varing_pos.set_col(nthVert, homo2Vec3(Viewport * Projection * ModelView * homoVec(world_coord)));
    }
    bool fragment(Vec3f factor, TGAColor &desColor)
    {
		//texture coord interpolate
        Vec2f interTex;
        interTex = varying_uv * factor;
        Vec3f bn = (varying_nrm*factor).normalize();

		//tangent normal mapping
        mat<3, 3, float> A;
        A[0] = varing_pos.col(1) - varing_pos.col(0);
        A[1] = varing_pos.col(2) - varing_pos.col(0);
        A[2] = bn;

        mat<3, 3, float> AI = A.invert();

        Vec3f i = AI * Vec3f(varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0);
        Vec3f j = AI * Vec3f(varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0);

        mat<3, 3, float> B;
        B.set_col(0, i.normalize());
        B.set_col(1, j.normalize());
        B.set_col(2, bn);

        Vec3f n = (B*model->normal(interTex)).normalize();

        float diff = std::max(0.f, n*light_dir);

		//shadow compute

		float shadow;
		Vec3f backShaowCoord = homo2Vec3(uniform_ShadowTran * homoVec(varing_pos*factor));
		if (!isValidScreenCoord(backShaowCoord, width, height))
			shadow = 1.0;
		else {
			int index = backShaowCoord.x + backShaowCoord.y * width;
			shadow = 0.3 + 0.7 *(backShaowCoord.z > shadowBuffer[index]);
		}
	
		//descolor compute
        desColor = model->diffuse(interTex)*diff * shadow;
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

class depthShader : public Ishader {

public:
    depthShader() {};
    void vertex(int iFace, int nthVert)
        {
            std::vector<Vec3i> face = model->face(iFace);

            Vec3f world_coord = model->vert(face[nthVert][0]);

            varing_pos.set_col(nthVert, homo2Vec3(Viewport * Projection * ModelView * homoVec(world_coord)));
        }
        bool fragment(Vec3f factor, TGAColor &desColor)
        {
            Vec3f p = varing_pos * factor;
            desColor = TGAColor(255, 255, 255, 255)*(p.z/depth);
            return false;
        }

        void sort() {
            //y increase by 0,1,2(col index)
            if (varing_pos[1][0] > varing_pos[1][1]) {
                varing_pos.swap_col(0, 1);

            }
            if (varing_pos[1][0] > varing_pos[1][2]) {
                varing_pos.swap_col(0, 2);
            }
            if (varing_pos[1][1] > varing_pos[1][2]) {
                varing_pos.swap_col(1, 2);
            }
        }
};

int main(int argc, char** argv) {
    TGAImage scene(width, height, TGAImage::RGB);
    TGAImage depthImage(width, height, TGAImage::RGB);

    zbuffer = new float[width * height];
    shadowBuffer = new float[width * height];
    for (int i = 0; i < width*height; i++) {
        zbuffer[i] = -std::numeric_limits<float>::max();
        shadowBuffer[i] = -std::numeric_limits<float>::max();
    }

#ifdef __APPLE__
    model = new Model("/Users/doris/Desktop/GIT/learnSoftRender/obj/diablo3_pose/diablo3_pose.obj");
    model->loadTexture("/Users/doris/Desktop/GIT/learnSoftRender/obj/diablo3_pose/diablo3_pose_diffuse.tga");
    model->loadNoraml("/Users/doris/Desktop/GIT/learnSoftRender/obj/diablo3_pose/diablo3_pose_nm_tangent.tga");
#elif _WIN32
    model = new Model("obj/diablo3_pose/diablo3_pose.obj");
    model->loadTexture("obj/diablo3_pose/diablo3_pose_diffuse.tga");
    model->loadNoraml("obj/diablo3_pose/diablo3_pose_nm_tangent.tga");
#endif

    lookat(light_dir, center, up);
    viewport(0, 0, width, height);
    Projection = Matrix::identity();

	Matrix M = Viewport * Projection * ModelView;


    depthShader dShader;
    for (int i = 0; i < model->nfaces(); i++) {
        for (int j = 0; j < 3; j++) {
            dShader.vertex(i, j);
        }
        dShader.sort();
        rasterization(depthImage, shadowBuffer, dShader);
    }

    depthImage.flip_vertically();
    depthImage.write_tga_file("depthImage.tga");

    std::cout << "Finish writing depth image.\n";

    lookat(cameraPos, center, up);
    viewport(0, 0, width, height);
    projection(cameraPos, center);
    Shader shader(Projection * ModelView, (Projection*ModelView).invert_transpose(),M * (Viewport * Projection * ModelView).invert());
    for (int i = 0; i < model->nfaces(); i++) {
        for (int j = 0; j < 3; j++) {
            shader.vertex(i, j);
        }
        shader.sort();
        rasterization(scene, zbuffer, shader);
    }
    delete model;
//
    scene.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    scene.write_tga_file("output.tga");
    delete[] zbuffer;
    delete[] shadowBuffer;
    /*std::cout << "yes" << std::endl;
     system("pause");*/
    return 0;
}

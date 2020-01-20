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
Vec3f eye = Vec3f(1, 1, 5);
Vec3f up = Vec3f(0, 1, 0);

TGAImage total(1024, 1024, TGAImage::GRAYSCALE);
TGAImage  occl(1024, 1024, TGAImage::GRAYSCALE);

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
        desColor = TGAColor(255, 255, 255, 255)*((p.z + 1.0) / 2);
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

class AOshader : public Ishader {
public:
    mat<2, 3, float> varying_uv;

public:
    AOshader() {};

    void vertex(int iFace, int nthVert)
    {
        std::vector<Vec3i> face = model->face(iFace);

        varying_uv.set_col(nthVert, model->text(face[nthVert][1]));
        Vec3f world_coord = model->vert(face[nthVert][0]);
        varing_pos.set_col(nthVert, homo2Vec3(Viewport * Projection * ModelView * homoVec(world_coord)));
    }
    bool fragment(Vec3f factor, TGAColor &desColor)
    {
        //texture coord interpolate
        Vec2f interTex;
        interTex = varying_uv * factor;
        Vec3f p = varing_pos * factor;
        if(std::abs(shadowBuffer[int(p.x+p.y*width)]-p.z)<1e-5)
        {
            occl.set(interTex.x * 1024, interTex.y * 1024, TGAColor(255, 255, 255));
        }
        //没什么用，主要是为了p适配fratgment Shader的作用
        //主要功能还是上面对occl光照贴图纹理进行写操作
        desColor = TGAColor(255, 0, 0);
        return false;
    }

    void sort() {
        //y increase by 0,1,2(col index)
        if (varing_pos[1][0] > varing_pos[1][1]) {
            varing_pos.swap_col(0, 1);
            varying_uv.swap_col(0, 1);
        }
        if (varing_pos[1][0] > varing_pos[1][2]) {
            varing_pos.swap_col(0, 2);
            varying_uv.swap_col(0, 2);
        }
        if (varing_pos[1][1] > varing_pos[1][2]) {
            varing_pos.swap_col(1, 2);
            varying_uv.swap_col(1, 2);
        }
    }
};

class phongShader : public Ishader {
public:
    mat<2, 3, float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
    mat<3, 3, float> varying_nrm; // normal per vertex to be interpolated by FS

    mat<4, 4, float> uniform_M;   //  Projection*ModelView
    mat<4, 4, float> uniform_MIT; // (Projection*ModelView).invert_transpose()
    mat<4, 4, float> uniform_ShadowTran;


public:
    phongShader() {};
    phongShader(Matrix M, Matrix MIT, Matrix shadowTrans) {
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

        Vec3f nMIT = homo2Vec3(uniform_MIT*homoVec(n)).normalize();
        Vec3f l = homo2Vec3(uniform_M  * homoVec(light_dir)).normalize();
        Vec3f r = (nMIT*(nMIT*l*2.f) - l).normalize();   // reflected light
        float spec = pow(std::max(r.z, 0.0f), model->specular(interTex));
        //descolor compute
        TGAColor co = model->ambient(interTex);
        TGAColor difCo = model->diffuse(interTex);
//        for(int j = 0; j < 3; j++)
//            desColor.raw[j] =  std::min<float>(co.b * 0.01 + (difCo.raw[j]*(0.8 * diff + 0.2 * spec ) * shadow), 255);
        //单纯环境光贴图
      desColor = TGAColor(co.b, co.b, co.b, 255);

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


Vec3f rand_point_on_unit_sphere() {
    float u = (float)rand()/(float)RAND_MAX;
    float v = (float)rand()/(float)RAND_MAX;
    float theta = 2.f*M_PI*u;
    float phi   = acos(2.f*v - 1.f);
    return Vec3f(sin(phi)*cos(theta), sin(phi)*sin(theta), cos(phi));
}


void resetZbfShadowBf()
{
    for (int i = 0; i < width*height; i++) {
           zbuffer[i] = -std::numeric_limits<float>::max();
           shadowBuffer[i] = -std::numeric_limits<float>::max();
       }
}

void computeAmbientMap()
{
    TGAImage depthImage(width, height, TGAImage::RGB);
    const int nrenders = 1000;
    for (int iter=1; iter<=nrenders; iter++) {
        std::cout << iter <<"/" << nrenders << std::endl;
        //随机出一个光源位置
        for (int i=0; i<3; i++) up[i] = (float)rand()/(float)RAND_MAX;
        eye = rand_point_on_unit_sphere();
        eye.y = std::abs(eye.y);
        std::cout << "eye " << eye << std::endl;
        lookat(eye, center, up);
        viewport(width/8, height/8, width*3/4, height*3/4);
        Projection = Matrix::identity();

        resetZbfShadowBf();

        //进行光照，计算各个区域深度，主要为了计算shadowBuffer
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
        depthImage.clear();

        //根据shadowBuffer内容，对这个点光源的纹理贴图进行赋值
        AOshader shader;
        occl.clear();
        for (int i = 0; i < model->nfaces(); i++) {
            for (int j = 0; j < 3; j++) {
                shader.vertex(i, j);
            }
            shader.sort();
            rasterization(depthImage, zbuffer, shader);
        }

        depthImage.flip_vertically();
        depthImage.write_tga_file("depthImageRRR.tga"); //这个图画出来的就是该点光源下的问题，不过所有可见处都是一个颜色，没有细节

        for (int i=0; i<1024; i++) {
            for (int j=0; j<1024; j++) {
                float tmp = total.get(i,j).raw[0];
                float c = (tmp*(iter-1)+occl.get(i,j).b)/(float)iter+.5f;
                total.set(i, j, TGAColor(c, c, c));
            }
        }
    }

    total.flip_vertically();
    total.write_tga_file("occlusion.tga");
    occl.flip_vertically();
    occl.write_tga_file("occl.tga");

    model->loadAmbient("occlusion.tga");
}


int main(int argc, char** argv)
{
    TGAImage scene(width, height, TGAImage::RGB);
    TGAImage depthImage(width, height, TGAImage::RGB);

    zbuffer = new float[width * height];
    shadowBuffer = new float[width * height];
    resetZbfShadowBf();

#ifdef __APPLE__
    model = new Model("/Users/doris/Desktop/GIT/learnSoftRender/obj/diablo3_pose/diablo3_pose.obj");
    model->loadTexture("/Users/doris/Desktop/GIT/learnSoftRender/obj/diablo3_pose/diablo3_pose_diffuse.tga");
    //model->loadAmbient("occlusion.tga");

    model->loadNoraml("/Users/doris/Desktop/GIT/learnSoftRender/obj/diablo3_pose/diablo3_pose_nm_tangent.tga");
    model->loadSpecular("/Users/doris/Desktop/GIT/learnSoftRender/obj/diablo3_pose/diablo3_pose_spec.tga");
#elif _WIN32
    model = new Model("obj/diablo3_pose/diablo3_pose.obj");
    model->loadTexture("obj/diablo3_pose/diablo3_pose_diffuse.tga");
    model->loadNoraml("obj/diablo3_pose/diablo3_pose_nm_tangent.tga");
#endif

    computeAmbientMap();
    eye = Vec3f(1, 1, 5);
    up = Vec3f(0, 1, 0);
    resetZbfShadowBf();
    depthImage.clear();

    //计算阴影
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
    depthImage.write_tga_file("renDepthImage.tga");
    std::cout << "Finish writing depth image.\n";

    //常规渲图
    lookat(eye, center, up);
    viewport(0, 0, width, height);
    projection(eye, center);
    phongShader shader(Projection * ModelView, (Projection*ModelView).invert_transpose(),M * (Viewport * Projection * ModelView).invert());
    for (int i = 0; i < model->nfaces(); i++) {
        for (int j = 0; j < 3; j++) {
            shader.vertex(i, j);
        }
        shader.sort();
        rasterization(scene, zbuffer, shader);
    }

    scene.flip_vertically();
    scene.write_tga_file("output.tga");


    delete[] zbuffer;
    delete[] shadowBuffer;
    delete model;
    return 0;
}

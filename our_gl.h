#pragma once
#include "geometry.h"
#include "tgaimage.h"

extern Matrix ModelView;
extern Matrix Viewport;
extern Matrix Projection;

void lookat(Vec3f eye, Vec3f center, Vec3f up);
void viewport(int x, int y, int w, int h);
void projection(Vec3f cameraPos, Vec3f center);
Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P);
void line(Vec2f p0, Vec2f p1, TGAImage &image, TGAColor color);
void triangle(Vec2f p0, Vec2f p1, Vec2f p2, TGAImage &image, TGAColor color);
//without textures
void rasterization(Vec3f p0, Vec3f p1, Vec3f p2, TGAImage &image, float *zBuffer, TGAColor color);
void rasterization(Vec3f p0, Vec3f p1, Vec3f p2, TGAImage &image, float *zBuffer, Vec3f norm_coord[], TGAImage &textureImage, Vec3f tex_coord[]);
Vec3f homo2Vec3(Vec4f h);
Vec4f homoVec(Vec3f v);
Vec4f homo2Vec4(Vec4f h);

#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec2f> texts_;
	std::vector<Vec3f> norms_;
	std::vector<std::vector<Vec3i> > faces_;
	TGAImage diffuseTexture_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	Vec2f text(int i);
	Vec3f norm(int idx);
	TGAColor diffuse(Vec2f uvf);
	std::vector<Vec3i> face(int idx);
	void loadTexture(std::string filename);
};

#endif //__MODEL_H__#pragma once

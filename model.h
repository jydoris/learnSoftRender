#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec3f> unitTex_;
	std::vector<std::vector<int> > faces_;
	std::vector<std::vector<int>> textures_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nTexs();
	int nfaces();
	Vec3f vert(int i);
	Vec3f text(int i);
	std::vector<int> face(int idx);
	std::vector<int> texures(int idx);
};

#endif //__MODEL_H__#pragma once

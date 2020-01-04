#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec3f> texts_;
	std::vector<Vec3f> norms_;
	std::vector<std::vector<Vec3i> > faces_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	Vec3f text(int i);
	Vec3f norm(int idx);
	std::vector<Vec3i> face(int idx);
};

#endif //__MODEL_H__#pragma once

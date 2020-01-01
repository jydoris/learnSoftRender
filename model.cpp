#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_() {
	std::ifstream in;
	in.open(filename, std::ifstream::in);
	if (in.fail()) return;
	std::string line;
	while (!in.eof()) {
		std::getline(in, line);
		std::istringstream iss(line.c_str());
		char trash;
		if (!line.compare(0, 2, "v ")) {
			iss >> trash;
			Vec3f v;
			for (int i = 0; i<3; i++) iss >> v[i];
			verts_.push_back(v);
		}
		else if (!line.compare(0, 3, "vt ")) {
			iss >> trash >> trash;
			Vec3f v;
			for (int i = 0; i<3; i++) iss >> v[i];
			unitTex_.push_back(v);
		}
		else if (!line.compare(0, 2, "f ")) {
			std::vector<int> f;
			std::vector<int> tIndex;
			int itrash, idx;
			int texId;
			iss >> trash;
			while (iss >> idx >> trash >> texId >> trash >> itrash) {
				idx--; // in wavefront obj all indices start at 1, not zero
				texId--;
				f.push_back(idx);
				tIndex.push_back(texId);
			}
			faces_.push_back(f);
			textures_.push_back(tIndex);
		}
	}
	std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
	return (int)verts_.size();
}

int Model::nTexs() {
	return (int)unitTex_.size();
}

int Model::nfaces() {
	return (int)faces_.size();
}

std::vector<int> Model::face(int idx) {
	return faces_[idx];
}

std::vector<int> Model::texures(int idx) {
	return textures_[idx];
}

Vec3f Model::vert(int i) {
	return verts_[i];
}

Vec3f Model::text(int i) {
	return unitTex_[i];
}
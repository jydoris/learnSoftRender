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
			Vec2f v;
			for (int i = 0; i<2; i++) iss >> v[i];
			texts_.push_back(v);
			iss >> trash;
		}
		else if (!line.compare(0, 3, "vn ")) {
			iss >> trash >> trash;
			Vec3f v;
			for (int i = 0; i<3; i++) iss >> v[i];
			norms_.push_back(v);
		}
		else if (!line.compare(0, 2, "f ")) {
			std::vector<Vec3i> f;
			int itrash;
			Vec3i tmp;
			iss >> trash;
			while (iss >> tmp[0] >> trash >> tmp[1] >> trash >> tmp[2]) {
				for (int i = 0; i < 3; i++) tmp[i]--;
				f.push_back(tmp);
			}
			faces_.push_back(f);
		}
	}
	std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
	return (int)verts_.size();
}

int Model::nfaces() {
	return (int)faces_.size();
}

std::vector<Vec3i> Model::face(int idx) {
	return faces_[idx];
}

void Model::loadTexture(std::string filename)
{
	diffuseTexture_.read_tga_file(filename.c_str());
	diffuseTexture_.flip_vertically();
}

void Model::loadNoraml(std::string filename)
{
	normalmap_.read_tga_file(filename.c_str());
	normalmap_.flip_vertically();
}

Vec3f Model::vert(int i) {
	return verts_[i];
}

Vec2f Model::text(int i) {
	return texts_[i];
}

Vec3f Model::norm(int i) {
	return norms_[i];
}

Vec3f Model::normal(Vec2f uvf) {
	Vec2i uv(uvf[0] * normalmap_.get_width(), uvf[1] * normalmap_.get_height());
	TGAColor c = normalmap_.get(uv[0], uv[1]);
	Vec3f res;
	for (int i = 0; i<3; i++)
		res[2 - i] = (float)c.raw[i] / 255.f*2.f - 1.f;
	return res;
}

TGAColor Model::diffuse(Vec2f uvf) {
	Vec2i uv(uvf[0] * diffuseTexture_.get_width(), uvf[1] * diffuseTexture_.get_height());
	return diffuseTexture_.get(uv[0], uv[1]);
}
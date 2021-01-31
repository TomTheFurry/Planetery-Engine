#pragma once
#include <glm/glm.hpp>
using vec3 = glm::vec3;

class RenderData {
public:
	RenderData() {}
	unsigned int vaoId;
	unsigned int veoId;
	int veoLength;
	glm::mat4* matrixs[5];
};


struct Light {
	vec3 pos;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float power;
};

class PlanetData {
public:
	PlanetData() {}
	vec3 pos;
	float size;
	unsigned int type;
};

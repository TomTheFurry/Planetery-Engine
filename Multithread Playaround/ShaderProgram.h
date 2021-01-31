#pragma once
#include <string>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <glfw/glfw3.h>

#include "Definer.h"

using str = std::string;

class ShaderProgram
{
public:
	uint id;

	ShaderProgram(str vertexPathP, str fragmentPathP);
	ShaderProgram(str vertexPathP, str geometryPathP, str fragmentPathP);
	void enable(); //set self as shader for rendering
	void setBool(str valueNameP, bool value) const;
	void setInt(str valueNameP, int value) const;
	void setFloat(str valueNameP, float value) const;
	void setFloatArray(str valueNameP, float* pointer, uint length) const;
	void setIvec2(str valueNameP, ivec2 value) const;
	void setVec3(str valueNameP, vec3 value) const;
	void setVec4(str valueNameP, vec4 value) const;
	void setMat3(str valueNameP, mat3 value) const;
	void setMat4(str valueNameP, mat4 value) const;

	~ShaderProgram();

	static void initClass();
	const static GLenum vaoMode;

private:
	bool shaderInit = false;
	bool hasGeoShader;
	str vPath, gPath, fPath;
	void loadFile();

	const static float boxVertix[];
};

#pragma once
#include <string>
#include <glm/glm.hpp>

using str = std::string;

class ShaderProgram
{
public:
	unsigned int id;

	ShaderProgram(str vertexPathP, str fragmentPathP);
	ShaderProgram(str vertexPathP, str geometryPathP, str fragmentPathP);
	void enable(); //set self as shader for rendering
	void setBool(str valueNameP, bool value) const;
	void setInt(str valueNameP, int value) const;
	void setFloat(str valueNameP, float value) const;
	void setVec3(str valueNameP, glm::vec3 value) const;
	void setVec4(str valueNameP, glm::vec4 value) const;
	void setMat3(str valueNameP, glm::mat3 value) const;
	void setMat4(str valueNameP, glm::mat4 value) const;
	~ShaderProgram();
private:
	bool shaderInit = false;
	bool hasGeoShader;
	str vPath, gPath, fPath;
	void loadFile();
};


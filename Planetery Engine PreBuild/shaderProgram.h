#pragma once
#include <string>

using str = std::string;

class ShaderProgram
{
public:
	unsigned int id;

	ShaderProgram(str vertexPathP, str fragmentPathP);
	void enable(); //set self as shader for rendering
	void setBool(str valueNameP, bool value) const;
	void setInt(str valueNameP, int value) const;
	void setFloat(str valueNameP, float value) const;
	void setVec(str valueNameP, float x, float y, float z, float w) const;
	~ShaderProgram();
private:
	bool shaderInit = false;
	str vPath, fPath;
	void loadFile();
};


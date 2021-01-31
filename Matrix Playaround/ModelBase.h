#pragma once

#include <Vector>

#include <glad/glad.h>
#include <glfw/glfw3.h>

#include "Global.h"

class ModelBase
{
public:
	ModelBase();
	virtual ~ModelBase();
	virtual uint gVao();
	virtual uint gVeo();
	virtual uint gLength();
	virtual GLenum gType();
};

class ModelCube : public ModelBase
{
public:
	ModelCube();
	~ModelCube();
	uint gVao();
	uint gVeo();
	uint gLength();
	GLenum gType();
	uint vaoId;
	uint vboId;
	uint veoId;
	uint length;

	const static float cubeVertices[];
    const static uint cubeIndices[];
	const static int cubeVertLength;
	const static int cubeIndLength;

};

class ModelGrid : public ModelBase
{
public:
	ModelGrid(uint size, float space, vec3 color);
	~ModelGrid();
	uint gVao();
	uint gVeo();
	uint gLength();
	GLenum gType();
	uint size;
	float space;

	std::vector<float> vertices;
	std::vector<uint> indices;

	uint vaoId;
	uint vboId;
	uint veoId;
	uint length;

};

class ModelPoints : public ModelBase
{
public:
	ModelPoints(uint count, vec3 centre, vec3 size, vec3 color);
	~ModelPoints();
	uint gVao();
	uint gVeo();
	uint gLength();
	GLenum gType();

	std::vector<float> vertices;
	std::vector<uint> indices;

	uint vaoId;
	uint vboId;
	uint length;

};
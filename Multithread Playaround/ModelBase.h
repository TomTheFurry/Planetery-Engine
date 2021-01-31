#pragma once

#include <Algorithm>
#include <Vector>

#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/perpendicular.hpp>
#include <PxPhysicsAPI.h>

#include "Definer.h"
#include "Translations.h"
#include "Physic.h"

/*
ModelBase.__constructor(): Run on any thread
ModelBase.glInit(): Run on render thread
ModelBase.render(): Run on render thread
ModelBase.__destructor(): Run on render thread
ModelBase.destruct(): Run on any thread

NOTE: Never delete ModelBase outside of render thread
NOTE: Always use "new"

ModelPhysic.pxUpdate(): Should run on physic thread

*/

class ActorStatic;
class ActorDynamic;
class Scene;

class ModelData
{
public:
	template <size_t V, size_t I>
	ModelData(const float(&v)[V], const uint(&i)[I]) {
		vert = v;
		ind = i;
		vertLength = V;
		indLength = I;
	}
	template <size_t V>
	ModelData(const float(&v)[V]) {
		vert = v;
		ind = nullptr;
		vertLength = V;
		indLength = 0;
	}
	const float* vert;
	const uint* ind;
	size_t vertLength;
	size_t indLength;
};

class ModelDataStatic : public ModelData
{
public:
	template <size_t V, size_t I>
	ModelDataStatic(const float(&v)[V], const uint(&i)[I]) : ModelData(v,i) {}
	template <size_t V>
	ModelDataStatic(const float(&v)[V]) : ModelData(v) {}
	void init();
	bool inited = false;
	uint vaoId = -1;
	uint vboId = -1;
	uint veoId = -1;
};

class ModelBase
{
public:
	ModelBase();
	virtual void destruct();
	virtual void glInit();
	virtual void glRender();
	virtual ~ModelBase();
	virtual uint gVao();
	virtual uint gVeo();
	virtual uint gLength();
	virtual mat4 gMat();
	virtual GLenum gType();
	virtual void renderUpdate(ulint delta);
};

class ModelPhysic : public ModelBase
{
public:
	ModelPhysic();
	virtual ~ModelPhysic();
	virtual void pxUpdate();
	physx::PxActor* actor;
};

class ModelStatic : public ModelPhysic, public TranslationStatic
{
public:
	ModelStatic();
	virtual ~ModelStatic();
	virtual void pxUpdate();
};

class ModelDynamic : public ModelPhysic, public TranslationDynamic
{
public:
	ModelDynamic();
	virtual ~ModelDynamic();
	virtual void pxUpdate();
	virtual void pxSleep();
	virtual void pxWake();
};

class ModelCube : public ModelDynamic {
public:
	ModelCube(Scene* scene);
	ModelCube(vec3 position, Scene* scene);
	void destruct();
	void glInit();
	void glRender();
	~ModelCube();
	uint gVao();
	uint gVeo();
	uint gLength();
	mat4 gMat();
	GLenum gType();
	void pxUpdate();
	void pxSleep();
	void pxWake();
	void renderUpdate(ulint delta);
	ActorDynamic* actor;

	static ModelDataStatic& modelData;

	bool isInside(vec3 position);
	bool isInside(vec3 position, vec3 boxSize);
};

class ModelConvex : public ModelDynamic {
public:
	ModelConvex(Scene* scene);
	ModelConvex(vec3 position, Scene* scene);
	void destruct();
	void glInit();
	void glRender();
	~ModelConvex();
	uint gVao();
	uint gVeo();
	uint gLength();
	mat4 gMat();
	GLenum gType();
	void pxUpdate();
	void pxSleep();
	void pxWake();
	void renderUpdate(ulint delta);
	uint vaoId;
	uint vboId;
	uint veoId;
	ActorDynamic* actor;

	static const ModelData& modelData;

	bool isInside(vec3 position);
	bool isInside(vec3 position, vec3 boxSize);
};

class ModelSphere : public ModelDynamic {
public:
	ModelSphere(Scene* scene);
	ModelSphere(vec3 position, Scene* scene);
	void destruct();
	void glInit();
	void glRender();
	~ModelSphere();
	uint gVao();
	uint gVeo();
	uint gLength();
	mat4 gMat();
	GLenum gType();
	void pxUpdate();
	void pxSleep();
	void pxWake();
	void renderUpdate(ulint delta);
	uint vaoId;
	uint vboId;
	uint veoId;
	ActorDynamic* actor;
	std::vector<float> vert;
	std::vector<uint> ind;
};

class ModelCubeStatic : public ModelStatic {
public:
	ModelCubeStatic(Scene* scene);
	ModelCubeStatic(vec3 position, Scene* scene);
	void destruct();
	void glInit();
	void glRender();
	~ModelCubeStatic();
	uint gVao();
	uint gVeo();
	uint gLength();
	mat4 gMat();
	GLenum gType();
	void pxUpdate();
	void renderUpdate(ulint delta);
	uint vaoId;
	uint vboId;
	uint veoId;
	ActorStatic* actor;

	static const ModelData& modelData;
};

class ModelCube2 : public ModelBase
{
public:
	ModelCube2();
	ModelCube2(vec3 position);
	void destruct();
	void glInit();
	void glRender();
	~ModelCube2();
	uint gVao();
	uint gVeo();
	uint gLength();
	mat4 gMat();
	GLenum gType();
	void pxUpdate(physx::PxActor* actor);
	void renderUpdate(ulint delta);
	uint vaoId;
	uint vboId;
	uint veoId;
	vec3 pos;

	static const ModelData& modelData;

	bool isInside(vec3 position);
	bool isInside(vec3 position, vec3 boxSize);
	bool isRayCollide(vec3 start, vec3 end);

};

class ModelGrid : public ModelBase
{
public:
	ModelGrid(uint size, float space, vec3 color);
	void destruct();
	void glInit();
	void glRender();
	~ModelGrid();
	uint gVao();
	uint gVeo();
	uint gLength();
	mat4 gMat();
	GLenum gType();
	void pxUpdate(physx::PxActor* actor);
	void renderUpdate(ulint delta);
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
	void destruct();
	void glInit();
	void glRender();
	~ModelPoints();
	uint gVao();
	uint gVeo();
	uint gLength();
	mat4 gMat();
	GLenum gType();
	void pxUpdate(physx::PxActor* actor);
	void renderUpdate(ulint delta);

	std::vector<float> vertices;
	std::vector<uint> indices;

	uint vaoId;
	uint vboId;
	uint length;

};

class ModelFloor : public ModelStatic
{
public:
	ModelFloor(Scene* scene);
	ModelFloor(vec3 position, vec3 normal, Scene* scene);
	void destruct();
	void glInit();
	void glRender();
	~ModelFloor();
	uint gVao();
	uint gLength();
	mat4 gMat();
	GLenum gType();
	void pxUpdate();
	void renderUpdate(ulint delta);
	uint vaoId;
	uint vboId;
	ActorStatic* actor;

	const static float floorVertices[];

};

class ModelGui : public ModelBase
{
public:
	const float CROSS_SIZE = 0.05f;

	ModelGui();
	void destruct();
	void glInit();
	void glRender();
	~ModelGui();
	uint gVao();
	uint gVeo();
	uint gLength();
	mat4 gMat();
	GLenum gType();
	void pxUpdate(physx::PxActor* actor);
	void renderUpdate(ulint delta);

	std::vector<float> lines;

	uint vaoId;
	uint vboId;
	uint length;
	
};
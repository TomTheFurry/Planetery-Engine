#pragma once
#include <glm/glm.hpp>
#include "Data.h"

using vec3 = glm::vec3;
using mat4 = glm::mat4;

class Camera
{
public:
	// Absolute Position
	// X: left->right  Y: back->front  Z: down->up

	// Relative Position
	// X: left->right  Y: back->front  Z: down->up

	// Rotation
	// X: yaw (XY)  Y: pitch (YZ)  Z: roll (XZ)
	Camera(float fovAngle, vec3 pos, vec3 rotate);
	bool doLockAngle = true;
	void setPosition(vec3 pos);
	void move(vec3 offset); //Relative movement
	void moveAbs(vec3 offset);
	void setRotation(vec3 rotate);
	void addRotation(vec3 rotate);
	void addRotationRelative(vec3 rotate);
	void lookAtPosition(vec3 pos);
	void lookAtPosition(vec3 pos, vec3 fromAngle);
	void setFov(float fovAngle);
	RenderData getRenderData();
	mat4* getViewportMatrix(); //only viewport
	mat4* getProjectedMatrix(); //viewport + projection
	mat4* getProjectionMatrix(); //only projection
	void updateMatrix();
	vec3 getPosition();
	vec3 getRotation();
protected:
	vec3 position;
	vec3 rotation; //yaw, pitch, roll
	float fov;
	bool needsUpdate;
	mat4 cachedProjectionMatrix;
	mat4 cachedProjectedMatrix;
	mat4 cachedViewportMatrix;
};


#include <glm/glm.hpp>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "Camera.h"


const float Camera::MAXACCEL = 0.2f;
const float Camera::MAXSPEED = 10.0f;

Camera::Camera(float fovAngle, vec3 pos, vec3 rotate) :
	TranslationTypeA()
{
	TranslationTypeA::pos = pos;
	TranslationTypeA::sRot(rotate);
	fov = fovAngle;
	maxAcc = MAXACCEL;
	maxVol = MAXSPEED;
}

mat4 Camera::getMatViewport()
{
	return gMatView();
}

mat4 Camera::getMatProject()
{
	return gMatProj();
}

void Camera::teleport(vec3 pos)
{
	sPos(pos);
}

void Camera::move(vec3 v)
{
	aAcc(v);
}

void Camera::stop()
{
	sVol(vec3(0.0f));
}

void Camera::setRotation(vec3 rotate)
{
	sRot(rotate);
}

void Camera::rotate(vec3 rotate)
{
	aRot(rotate);
}

void Camera::update()
{
	TranslationTypeA::update();
}
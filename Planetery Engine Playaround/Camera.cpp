#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "Camera.h"
#include "Data.h"
#include "Global.h"

using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;

Camera::Camera(float fovAngle, vec3 pos, vec3 rotate)
{
	fov = fovAngle;
	position = pos;
	rotation = rotate;
	needsUpdate = true;
}

void Camera::setPosition(vec3 pos)
{
	position = pos;
	needsUpdate = true;
}

void Camera::move(vec3 offset)
{
	position.z += offset.z;
	//yaw 0 = +y
	//yaw 90 = +x
	//yaw 180 = -y
	//yaw 270 = -x
	position.x += sin(glm::radians(rotation.x)) * offset.y;
	position.y += cos(glm::radians(rotation.x)) * offset.y;
	//yaw 0 = +x
	//yaw 90 = -y
	//yaw 180 = -x
	//yaw 270 = +y
	position.x += cos(glm::radians(rotation.x)) * offset.x;
	position.y += -sin(glm::radians(rotation.x)) * offset.x;

	needsUpdate = true;
}

void Camera::moveAbs(vec3 offset)
{
	position += offset;
	needsUpdate = true;
}

void Camera::setRotation(vec3 rotate)
{
	rotation = rotate;
	needsUpdate = true;
}

void Camera::addRotation(vec3 rotate)
{
	rotation += rotate;
	needsUpdate = true;
}

void Camera::addRotationRelative(vec3 rotate)
{
	auto r = glm::rotate(mat4(1.0f), glm::radians(rotation.x), vec3(0.0f, 0.0f, -1.0f));
	r = glm::rotate(r, glm::radians(rotation.y), vec3(1.0f, 0.0f, 0.0f));
	r = glm::rotate(r, glm::radians(rotation.z), vec3(0.0f, -1.0f, 0.0f));

	auto r2 = glm::rotate(mat4(1.0f), glm::radians(rotate.x), vec3(0.0f, 0.0f, -1.0f));
	r2 = glm::rotate(r2, glm::radians(rotate.y), vec3(1.0f, 0.0f, 0.0f));
	r2 = glm::rotate(r2, glm::radians(rotate.z), vec3(0.0f, -1.0f, 0.0f));

}

void Camera::lookAtPosition(vec3 pos)
{
	//something
	needsUpdate = true;
}

void Camera::lookAtPosition(vec3 pos, vec3 fromAngle)
{
	//something
	needsUpdate = true;
}

void Camera::setFov(float fovAngle)
{
	fov = fovAngle;
	needsUpdate = true;
}

RenderData Camera::getRenderData()
{
	auto r = RenderData();
	r.matrixs[0] = getViewportMatrix();
	r.matrixs[1] = getProjectedMatrix();

	return r;
}


mat4* Camera::getViewportMatrix()
{
	if (needsUpdate) {
		updateMatrix();
	}
	return &cachedViewportMatrix;
}


mat4* Camera::getProjectedMatrix()
{
	if (needsUpdate) {
		updateMatrix();
	}
	return &cachedProjectedMatrix;

}

mat4* Camera::getProjectionMatrix()
{
	if (needsUpdate) {
		updateMatrix();
	}
	return &cachedProjectionMatrix;
}

void Camera::updateMatrix()
{
	if (doLockAngle) {
		if (rotation.y > 89.9f) rotation.y = 89.9f;
		if (rotation.y < -89.9f) rotation.y = -89.9f;
	}

	cachedViewportMatrix = glm::rotate(mat4(1.0f), glm::radians(rotation.x), vec3(0.0f, 0.0f, -1.0f));
	cachedViewportMatrix = glm::rotate(cachedViewportMatrix, glm::radians(rotation.y), vec3(1.0f, 0.0f, 0.0f));
	cachedViewportMatrix = glm::rotate(cachedViewportMatrix, glm::radians(rotation.z), vec3(0.0f, -1.0f, 0.0f));
	vec3 target = (vec3)(cachedViewportMatrix * vec4(0.0f, 1.0f, 0.0f, 1.0f));
	vec3 up = (vec3)(cachedViewportMatrix * vec4(0.0f, 0.0f, 1.0f, 1.0f));
	cachedViewportMatrix = glm::lookAt(position, position + target, up);
	cachedProjectionMatrix = glm::perspective(glm::radians(fov), global->windowRatio(), 0.0001f, 1000.0f);
	cachedProjectedMatrix = (cachedProjectionMatrix * cachedViewportMatrix);
	needsUpdate = false;
}

vec3 Camera::getPosition()
{
	return position;
}

vec3 Camera::getRotation()
{
	return rotation;
}

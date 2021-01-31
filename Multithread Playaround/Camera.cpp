#include <glm/glm.hpp>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "Camera.h"

#include "Global.h"

CameraBase::CameraBase() {
	fov = global->fov;
	viewportSize = vec4(global->windowSize, 0.01f, 100.0f);
}
CameraBase::~CameraBase() {}
mat4 CameraBase::getMatViewport()
{
	mat4 matRot = _EulerMat(t->gRot());
	vec3 target = (vec3)(matRot * vec4(0.0f, 1.0f, 0.0f, 1.0f));
	vec3 up = (vec3)(matRot * vec4(0.0f, 0.0f, 1.0f, 1.0f));
	return glm::lookAt(t->gPos(), t->gPos() + target, up);
}
mat4 CameraBase::getMatProject()
{
	return glm::perspective(glm::radians(fov), viewportSize.x/viewportSize.y, viewportSize.z, viewportSize.w);
}

vec3 CameraBase::gPos()
{
	return t->gPos();
}

vec3 CameraBase::gRot()
{
	return t->gRot();
}

void CameraBase::update()
{
}



CameraMovable::CameraMovable() : CameraBase() {}
CameraMovable::~CameraMovable() {}
void CameraMovable::move(vec3 v) { t->aPos(v); }
void CameraMovable::teleport(vec3 v) { t->sPos(v); }
void CameraMovable::rotate(vec3 r) { t->aRot(r); }
void CameraMovable::setRotate(vec3 r) { t->sRot(r); }



CameraFloating::CameraFloating() : CameraMovable() { drag = 0.01f; }
CameraFloating::~CameraFloating() {}
void CameraFloating::move(vec3 v) { t->aVol(v); }
void CameraFloating::teleport(vec3 v) { t->sPos(v); }
void CameraFloating::slowdown(float p) { t->sVol(vec3(0.f)); }
void CameraFloating::rotate(vec3 r) { t->aRot(r); }
void CameraFloating::setRotate(vec3 r) { t->sRot(r); }
void CameraFloating::update() {
	t->sVol(t->gVol()*(1-drag));
	t->update();
}



const float CameraFreeStyle::MAXACCEL = 0.2f;
const float CameraFreeStyle::MAXSPEED = 10.0f;

CameraFreeStyle::CameraFreeStyle()
{
	t = new TranslationTypeA();
	((TranslationTypeA*)t)->maxAcc = MAXACCEL;
	((TranslationTypeA*)t)->maxVol = MAXSPEED;
}

CameraFreeStyle::~CameraFreeStyle()
{
	delete t;
}

mat4 CameraFreeStyle::getMatViewport()
{
	vec3 target = (vec3)(((TranslationTypeA*)t)->matRot * vec4(0.0f, 1.0f, 0.0f, 1.0f));
	vec3 up = (vec3)(((TranslationTypeA*)t)->matRot * vec4(0.0f, 0.0f, 1.0f, 1.0f));
	mat4 m = glm::lookAt(t->gPos(), t->gPos() + target, up);
	
	return m;
}



CameraTargetMovable::CameraTargetMovable()
{
	t = new TranslationBasic();
	distance = 10.f;
}

CameraTargetMovable::~CameraTargetMovable()
{
	delete t;
}

mat4 CameraTargetMovable::getMatViewport()
{
	mat4 matRot = _EulerMat(((TranslationBasic*)t)->gRot());

	vec3 target = matRot * vec4(0.0f, distance, 0.0f, 1.0f);
	vec3 up = matRot * vec4(0.0f, 0.0f, 1.0f, 1.0f);
	mat4 m = glm::lookAt(t->gPos() - target, t->gPos(), up);
	return m;
}

void CameraTargetMovable::move(vec3 v) {
	vec3 rota = vec3(((TranslationBasic*)t)->gRot().x, 0.f, 0.f);
	v = _EulerMat(rota) * vec4(v * 50.f, 0.f);
	t->aPos(v);
}

void CameraTargetMovable::update()
{
	t->update();
	if (updateCallback != nullptr) updateCallback(gPos());
}

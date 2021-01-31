#pragma once
#include "Definer.h"

#include "Translations.h"

class CameraBase
{
public:
	TranslationBase* t;
	float fov;
	vec4 viewportSize; //x,y, minZ, maxZ

	CameraBase();
	virtual ~CameraBase();
	virtual mat4 getMatViewport();
	virtual mat4 getMatProject();
	virtual vec3 gPos();
	virtual vec3 gRot();
	virtual void update();
};

class CameraMovable : public CameraBase
{
public:
	CameraMovable();
	virtual ~CameraMovable();
	virtual void move(vec3 v);
	virtual void teleport(vec3 v);
	virtual void rotate(vec3 r);
	virtual void setRotate(vec3 r);
};

class CameraFloating : public CameraMovable
{
public:
	float drag;
	CameraFloating();
	virtual ~CameraFloating();
	virtual void move(vec3 v);
	virtual void teleport(vec3 v);
	virtual void slowdown(float p);
	virtual void rotate(vec3 r);
	virtual void setRotate(vec3 r);
	virtual void update();
};

class CameraFreeStyle : public CameraFloating
{
public:
	const static float MAXACCEL;
	const static float MAXSPEED;
	CameraFreeStyle();
	virtual ~CameraFreeStyle();
	mat4 getMatViewport();
};

class CameraTargetMovable : public CameraMovable
{
public:
	float distance;
	void(*updateCallback)(vec3) = nullptr;
	CameraTargetMovable();
	virtual ~CameraTargetMovable();
	void move(vec3 v);
	mat4 getMatViewport();
	void update();
};


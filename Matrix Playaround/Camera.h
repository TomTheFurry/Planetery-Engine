#pragma once
#include "Global.h"

#include "Translations.h"



class Camera : public TranslationTypeA
{
public:
	const static float MAXACCEL;
	const static float MAXSPEED;
	Camera(float fovAngle, vec3 pos, vec3 rotate);
	mat4 getMatViewport();
	mat4 getMatProject();
	void teleport(vec3 v);
	void move(vec3 v);
	void stop();
	void setRotation(vec3 v);
	void rotate(vec3 v);
	void update();
};


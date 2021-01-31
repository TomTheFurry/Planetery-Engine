#pragma once

#include "utility.h"



class TranslationBase
{
public:
	TranslationBase();
	virtual vec3 gPos();
	virtual void sPos(vec3 v);
	virtual void aPos(vec3 v);

	virtual vec3 gVol();
	virtual void sVol(vec3 v);
	virtual void aVol(vec3 v);

	virtual vec3 gRot();
	virtual void sRot(vec3 v);
	virtual void aRot(vec3 v);

	virtual void update();
};

class TranslationBasic : public TranslationBase
{
public:
	vec3 pos;
	vec3 vol;
	vec3 rot;

	TranslationBasic();

	vec3 gPos();
	void sPos(vec3 v);
	void aPos(vec3 v);

	vec3 gVol();
	void sVol(vec3 v);
	void aVol(vec3 v);

	vec3 gRot();
	void sRot(vec3 v);
	void aRot(vec3 v);

	void update();
};

class TranslationRotateA : public TranslationBase
{
public:
	vec3 pos;
	vec3 vol;
	mat4 matRot;
	float fov;
	float min;
	float max;

	TranslationRotateA();

	vec3 gPos();
	void sPos(vec3 v);
	void aPos(vec3 v);

	vec3 gVol();
	void sVol(vec3 v);
	void aVol(vec3 v);

	vec3 gRot();
	void sRot(vec3 v);
	void aRot(vec3 v);

	void update();

	mat4 gMatView();

	mat4 gMatProj();
};

class TranslationTypeA : public TranslationRotateA
{
public:
	float maxAcc;
	vec3 tickAcc;
	float maxVol;

	TranslationTypeA();

	vec3 gVol();
	void sVol(vec3 v);
	void aVol(vec3 v);

	vec3 gAcc();
	void sAcc(vec3 v);
	void aAcc(vec3 v);

	void update();

	void sMaxAcc(float v);
	float gMaxAcc();

	void sMaxVol(float v);
	float gMaxVol();

};
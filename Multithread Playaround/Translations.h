#pragma once

#include "Definer.h"



class TranslationBase
{
public:
	TranslationBase();
	virtual ~TranslationBase();
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
	virtual ~TranslationBasic();

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
	virtual ~TranslationRotateA();

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

class TranslationTypeA : public TranslationRotateA
{
public:
	float maxAcc;
	vec3 tickAcc;
	float maxVol;

	TranslationTypeA();
	virtual ~TranslationTypeA();

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

class TranslationPhysicBase
{
public:
	TranslationPhysicBase();
	virtual ~TranslationPhysicBase();

	virtual vec3 gPos();
	virtual void sPos(vec3 v);
	virtual vec3 gPosV();
	virtual void sPosV(vec3 v);
	virtual vec3 gRot();
	virtual void sRot(vec3 v);
	virtual vec3 gRotV();
	virtual void sRotV(vec3 v);

	virtual void update(ulint delta);
};

class TranslationStatic : public TranslationPhysicBase
{
public:
	vec3 pos;
	vec3 rot;

	TranslationStatic();
	virtual ~TranslationStatic();
	virtual vec3 gPos();
	virtual void sPos(vec3 v);

	virtual vec3 gRot();
	virtual void sRot(vec3 v);

	virtual void sync(vec3 p, vec3 r);
};

class TranslationDynamic : public TranslationPhysicBase
{
public:
	vec3 pos;
	vec3 rot;
	vec3 posV;
	vec3 rotV;

	TranslationDynamic();
	virtual ~TranslationDynamic();
	virtual vec3 gPos();
	virtual void sPos(vec3 v);
	virtual vec3 gPosV();
	virtual void sPosV(vec3 v);
	virtual vec3 gRot();
	virtual void sRot(vec3 v);
	virtual vec3 gRotV();
	virtual void sRotV(vec3 v);

	virtual void update(ulint delta);
	virtual void sync(vec3 p, vec3 r, vec3 pv, vec3 rv);
};
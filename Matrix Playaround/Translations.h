#pragma once


using uvec2 = glm::u32vec2;
using uvec3 = glm::u32vec3;
using uvec4 = glm::u32vec4;
using ivec2 = glm::i32vec2;
using ivec3 = glm::i32vec3;
using ivec4 = glm::i32vec4;
using lvec2 = glm::i64vec2;
using lvec3 = glm::i64vec3;
using lvec4 = glm::i64vec4;
using vec2 = glm::f32vec2;
using vec3 = glm::f32vec3;
using vec4 = glm::f32vec4;
using dvec2 = glm::f64vec2;
using dvec3 = glm::f64vec3;
using dvec4 = glm::f64vec4;
using mat2 = glm::f32mat2;
using mat3 = glm::f32mat3;
using mat4 = glm::f32mat4;

using uint = unsigned int;
using ulint = unsigned long long;
using lint = long long;



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
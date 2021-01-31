
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "Translations.h"

#include "Global.h"



TranslationBase::TranslationBase() {}
TranslationBase::~TranslationBase()
{
}
; //Does nothing
vec3 TranslationBase::gPos() { return vec3(-1.0f); } //Override for get Pos
void TranslationBase::sPos(vec3 v) {}; //Override for set Pos
void TranslationBase::aPos(vec3 v) {}; //Override for set Pos

vec3 TranslationBase::gVol() { return vec3(-1.0f); }; //Override for get Vol (Speed)
void TranslationBase::sVol(vec3 v) {}; //Override for set Vol (Speed)
void TranslationBase::aVol(vec3 v) {}; //Override for set Vol (Speed)

vec3 TranslationBase::gRot() { return vec3(-1.0f); }; //Override for get Vol (Speed)
void TranslationBase::sRot(vec3 v) {}; //Override for set Vol (Speed)
void TranslationBase::aRot(vec3 v) {}; //Override for set Vol (Speed)

void TranslationBase::update() {};



TranslationBasic::TranslationBasic() : TranslationBase() {
	pos = vec3(0.0f);
	vol = vec3(0.0f);
	rot = vec3(0.0f);
}

TranslationBasic::~TranslationBasic()
{
}

vec3 TranslationBasic::gPos() {
	return pos;
}
void TranslationBasic::sPos(vec3 v) {
	pos = v;
}
void TranslationBasic::aPos(vec3 v) {
	pos += v;
}

vec3 TranslationBasic::gVol() {
	return vol;
}
void TranslationBasic::sVol(vec3 v) {
	vol = v;
}
void TranslationBasic::aVol(vec3 v) {
	vol += v;
}

// Rotation
// X: yaw (XY)(left to right)  Y: pitch (YZ)(down to up)  Z: roll (XZ)(clockrise)

vec3 TranslationBasic::gRot() {
	return rot;
}
void TranslationBasic::sRot(vec3 v) {
	rot = v;
}
void TranslationBasic::aRot(vec3 v) {
	rot += v;
	
	rot.x = remainder(rot.x, 360.f);
	rot.z = remainder(rot.z, 360.f);
	if (rot.y > 90.f) rot.y = 90.f;
	if (rot.y < -90.f) rot.y = -90.f;
}

void TranslationBasic::update() {
	pos += vol;
}



TranslationRotateA::TranslationRotateA() : TranslationBase() {
	pos = vec3(0.0f);
	vol = vec3(0.0f);
	matRot = mat4(1.0f);
	fov = 60.0f;
	max = 100.0f;
	min = 0.01f;
}

TranslationRotateA::~TranslationRotateA()
{
}

vec3 TranslationRotateA::gPos() {
	return pos;
}
void TranslationRotateA::sPos(vec3 v) {
	pos = v;
}
void TranslationRotateA::aPos(vec3 v) {
	pos += vec3(matRot * vec4(v, 0.0f));
}

vec3 TranslationRotateA::gVol() {
	return vol;
}
void TranslationRotateA::sVol(vec3 v) {
	vol = v;
}
void TranslationRotateA::aVol(vec3 v) {
	vol += vec3(matRot * vec4(v, 0.0f));
}

// Rotation
// X: yaw (XY)(left to right)  Y: pitch (YZ)(down to up)  Z: roll (XZ)(clockrise)

vec3 TranslationRotateA::gRot() {
	return _MatEuler(matRot);
}

void TranslationRotateA::sRot(vec3 v) {
	matRot = _EulerMat(v);
}
void TranslationRotateA::aRot(vec3 v) {
	matRot *= _EulerMat(v);
}

void TranslationRotateA::update() {
	pos += vol;
}



TranslationTypeA::TranslationTypeA() : TranslationRotateA() {
	tickAcc = vec3(0.0f);
	maxAcc = 0.0f;
	maxVol = 100.0f;
}
TranslationTypeA::~TranslationTypeA()
{
}
vec3 TranslationTypeA::gVol() {
	auto v = vol + gAcc();
	if (glm::length(v) < maxVol) return v;
	else return _normalize(v) * maxVol;
}
void TranslationTypeA::sVol(vec3 v) {
	tickAcc += (v - gVol());
}
void TranslationTypeA::aVol(vec3 v) {
	tickAcc += vec3(matRot * vec4(v, 0.0f));
}

vec3 TranslationTypeA::gAcc() {
	if (glm::length(tickAcc) < maxAcc) return tickAcc;
	else return _normalize(tickAcc) * maxAcc;
}
void TranslationTypeA::sAcc(vec3 v) {
	tickAcc = v;
}
void TranslationTypeA::aAcc(vec3 v) {
	tickAcc += vec3(matRot * vec4(v, 0.0f));
} //Same as aVol()

void TranslationTypeA::update() {
	vol += gAcc();
	if (glm::length(vol) >= maxVol)
		vol = _normalize(vol) * maxVol;
	tickAcc = vec3(0.0f);
	pos += vol;
}

void TranslationTypeA::sMaxAcc(float v) {
	maxAcc = abs(v);
}
float TranslationTypeA::gMaxAcc() {
	return maxAcc;
}

void TranslationTypeA::sMaxVol(float v) {
	maxVol = abs(v);
}
float TranslationTypeA::gMaxVol() {
	return maxVol;
}



TranslationPhysicBase::TranslationPhysicBase() {}
TranslationPhysicBase::~TranslationPhysicBase()
{
}
vec3 TranslationPhysicBase::gPos() {return vec3(0.f);}
void TranslationPhysicBase::sPos(vec3 v) {}
vec3 TranslationPhysicBase::gPosV() {return vec3(0.f);}
void TranslationPhysicBase::sPosV(vec3 v) {}
vec3 TranslationPhysicBase::gRot() {return vec3(0.f);}
void TranslationPhysicBase::sRot(vec3 v){}
vec3 TranslationPhysicBase::gRotV() {return vec3(0.f);}
void TranslationPhysicBase::sRotV(vec3 v){}
void TranslationPhysicBase::update(ulint delta){}



TranslationStatic::TranslationStatic() {
	pos = vec3(0);
	rot = vec3(0);
}

TranslationStatic::~TranslationStatic()
{
}

vec3 TranslationStatic::gPos() { return pos; }
void TranslationStatic::sPos(vec3 v) { pos = v; }
vec3 TranslationStatic::gRot() { return rot; }
void TranslationStatic::sRot(vec3 v) { rot = v; }

void TranslationStatic::sync(vec3 pos, vec3 rot) {
	sPos(pos);
	sRot(rot);
}



TranslationDynamic::TranslationDynamic(){
	pos = vec3(0);
	rot = vec3(0);
	posV = vec3(0);
	rotV = vec3(0);
}

TranslationDynamic::~TranslationDynamic()
{
}

vec3 TranslationDynamic::gPos() {return pos;}
void TranslationDynamic::sPos(vec3 v) {pos = v;}
vec3 TranslationDynamic::gPosV() {return posV;}
void TranslationDynamic::sPosV(vec3 v) {posV = v;}
vec3 TranslationDynamic::gRot() {return rot;}
void TranslationDynamic::sRot(vec3 v) {rot = v;}
vec3 TranslationDynamic::gRotV() {return rotV;}
void TranslationDynamic::sRotV(vec3 v) {rotV = v;}

void TranslationDynamic::update(ulint delta) {
	pos += (posV * float(delta)) / 1000000000.f;
	rot += (rotV * float(delta)) / 1000000000.f;
}

void TranslationDynamic::sync(vec3 pos, vec3 rot, vec3 posV, vec3 rotV) {
	sPos(pos);
	sPosV(posV);
	sRot(rot);
	sRotV(rotV);
}
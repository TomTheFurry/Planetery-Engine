
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Translations.h"

#include "Global.h"



TranslationBase::TranslationBase() {}; //Does nothing
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

vec3 TranslationBasic::gRot() {
	return rot;
}
void TranslationBasic::sRot(vec3 v) {
	rot = v;
}
void TranslationBasic::aRot(vec3 v) {
	rot = v;
}

void TranslationBasic::update() {
	pos += vol;
}



TranslationRotateA::TranslationRotateA() : TranslationBase() {
	pos = vec3(0.0f);
	vol = vec3(0.0f);
	matRot = mat4(1.0f);
	fov = 60.0f;
	max = global->farView;
	min = global->nearView;
}

vec3 TranslationRotateA::gPos() {
	return pos;
}
void TranslationRotateA::sPos(vec3 v) {
	pos = v;
}
void TranslationRotateA::aPos(vec3 v) {
	pos += v;
}

vec3 TranslationRotateA::gVol() {
	return vol;
}
void TranslationRotateA::sVol(vec3 v) {
	vol = v;
}
void TranslationRotateA::aVol(vec3 v) {
	vol += v;
}

vec3 TranslationRotateA::gRot() {
	return vec3(0.0f); //TODO: Somehow get a rotation??
}
void TranslationRotateA::sRot(vec3 v) {
	matRot = glm::rotate(mat4(1.0f), glm::radians(v.x), vec3(0.0f, 0.0f, -1.0f));
	matRot = glm::rotate(matRot, glm::radians(v.y), vec3(1.0f, 0.0f, 0.0f));
	matRot = glm::rotate(matRot, glm::radians(v.z), vec3(0.0f, -1.0f, 0.0f));
}
void TranslationRotateA::aRot(vec3 v) {
	auto r = glm::rotate(mat4(1.0f), glm::radians(v.x), vec3(0.0f, 0.0f, -1.0f));
	r = glm::rotate(r, glm::radians(v.y), vec3(1.0f, 0.0f, 0.0f));
	r = glm::rotate(r, glm::radians(v.z), vec3(0.0f, -1.0f, 0.0f));
	matRot = matRot * r;
}

void TranslationRotateA::update() {
	pos += vol;
}

mat4 TranslationRotateA::gMatView() {
	vec3 target = (vec3)(matRot * vec4(0.0f, 1.0f, 0.0f, 1.0f));
	vec3 up = (vec3)(matRot * vec4(0.0f, 0.0f, 1.0f, 1.0f));
	return glm::lookAt(pos, pos + target, up);
}

mat4 TranslationRotateA::gMatProj() {
	return glm::perspective(glm::radians(fov), global->windowRatio(), min, max);
}



TranslationTypeA::TranslationTypeA() : TranslationRotateA() {
	tickAcc = vec3(0.0f);
	maxAcc = 0.0f;
	maxVol = 100.0f;
}
vec3 TranslationTypeA::gVol() {
	auto v = vol + gAcc();
	if (glm::length(v) < maxVol) return v;
	else return glm::normalize(v) * maxVol;
}
void TranslationTypeA::sVol(vec3 v) {
	tickAcc += (v - gVol());
}
void TranslationTypeA::aVol(vec3 v) {
	tickAcc += vec3(matRot * vec4(v, 0.0f));
}

vec3 TranslationTypeA::gAcc() {
	if (glm::length(tickAcc) < maxAcc) return tickAcc;
	else return glm::normalize(tickAcc) * maxAcc;
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
		vol = glm::normalize(vol) * maxVol;
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
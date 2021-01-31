#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "PxFoundations.h"

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
using bvec2 = glm::bvec2;
using bvec3 = glm::bvec3;
using bvec4 = glm::bvec4;
using dvec2 = glm::f64vec2;
using dvec3 = glm::f64vec3;
using dvec4 = glm::f64vec4;
using mat2 = glm::f32mat2;
using mat3 = glm::f32mat3;
using mat4 = glm::f32mat4;
using dmat2 = glm::f64mat2;
using dmat3 = glm::f64mat3;
using dmat4 = glm::f64mat4;
using quat = glm::quat;

using uint = unsigned int;
using ulint = unsigned long long;
using lint = long long;

// Absolute Position
// X: left->right  Y: back->front  Z: down->up

// Relative Position
// X: left->right  Y: back->front  Z: down->up

// Rotation
// X: yaw (XY)(left to right)  Y: pitch (YZ)(down to up)  Z: roll (XZ)(clockrise)

inline mat4 _AnglarVToRotMat(physx::PxVec3 w) {
	return mat4(vec4(   1, w.z,-w.y, 0),
				vec4(-w.z,   1, w.x, 0),
				vec4( w.y,-w.x,   1, 0),
				vec4(   0,   0,   0, 1));
}



inline mat4 _EulerMat(vec3 e) {
	auto matRot = glm::rotate(mat4(1.0f), glm::radians(e.x), vec3(0.0f, 0.0f, -1.0f));
	matRot = glm::rotate(matRot, glm::radians(e.y), vec3(1.0f, 0.0f, 0.0f));
	matRot = glm::rotate(matRot, glm::radians(e.z), vec3(0.0f, 1.0f, 0.0f));
	return matRot;
}
inline vec3 _MatEuler(mat4 m) {
	float x, y, z;
	glm::extractEulerAngleZXY(m, x, y, z);
	return glm::degrees(vec3(-x, y, z));
}

inline quat _EulerQuat(vec3 e) {
	return glm::toQuat(_EulerMat(e));
}
inline vec3 _QuatEuler(quat q) {
	return _MatEuler(mat4(q));
}



inline physx::PxQuat _EulerPXQuat(vec3 e) {
	auto q = glm::toQuat(_EulerMat(e));
	return physx::PxQuat(q.x, q.y, q.z, q.w);
}

inline vec3 _PXQuatEuler(physx::PxQuat q) {
	return _MatEuler(mat4(quat(q.w, q.x, q.y, q.z)));
}




inline physx::PxVec2 _CastGLPX(vec2 v) {
	return physx::PxVec2(v.x, v.y);
}
inline vec2 _CastGLPX(physx::PxVec2 v) {
	return vec2(v.x, v.y);
}
inline physx::PxVec3 _CastGLPX(vec3 v) {
	return physx::PxVec3(v.x, v.y, v.z);
}
inline vec3 _CastGLPX(physx::PxVec3 v) {
	return vec3(v.x, v.y, v.z);
}
inline physx::PxVec4 _CastGLPX(vec4 v) {
	return physx::PxVec4(v.x, v.y, v.z, v.w);
}
inline vec4 _CastGLPX(physx::PxVec4 v) {
	return vec4(v.x, v.y, v.z, v.w);
}
inline physx::PxQuat _CastGLPXrot(vec3 v) {
	return _EulerPXQuat(v);
}
inline vec3 _CastGLPXrot(physx::PxQuat v) {
	return _PXQuatEuler(v);
}
inline vec3 _CastGLPXrotVol(physx::PxVec3 v) {
	return _MatEuler(_AnglarVToRotMat(v));
}

//Fixed the Nan for vec3(0.0f) issue
inline vec3 _normalize(vec3 v) {
	if (v == vec3(0.f)) return vec3(0.f);
	return glm::normalize(v);
}
#define NS_PER_H 360000000000
#define NS_PER_H_F 360000000000.
#define NS_PER_M 60000000000
#define NS_PER_M_F 60000000000.
#define NS_PER_S 1000000000
#define NS_PER_S_F 1000000000.
#define NS_PER_MS 1000000
#define NS_PER_MS_F 1000000.
#define NS_PER_US 1000
#define NS_PER_US_F 1000.

template <typename T>
std::string nanoSec(T ns) {
	if (ns < NS_PER_US)	return std::to_string(ns) + "ns";
	if (ns < NS_PER_MS)	return std::to_string(ns / NS_PER_US_F) + "us";
	if (ns < NS_PER_S)	return std::to_string(ns / NS_PER_MS_F) + "ms";
	if (ns < NS_PER_M)	return std::to_string(ns /  NS_PER_S_F) + "s";
						return std::to_string(ns /  NS_PER_M_F) + "min";
}

template <typename T>
std::string sec(T s) {
	return nanoSec(s * NS_PER_S);
}


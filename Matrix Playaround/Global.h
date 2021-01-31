#pragma once
#include <fstream>

#include <glm/glm.hpp>

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
using dmat2 = glm::f64mat2;
using dmat3 = glm::f64mat3;
using dmat4 = glm::f64mat4;

using uint = unsigned int;
using ulint = unsigned long long;
using lint = long long;

#include "Camera.h"

void printMatrix(glm::mat4 mat);
void printVec3(glm::vec3 vec);
float gaussianFunction(float offset, float mean, float deviation);

class Camera;

class Global {
public:
	ivec2 windowSize = ivec2(720,1080);
	float fov = 60.0f;
	float windowRatio();
	int targetTps = 60;
	Camera* activeCamera = nullptr;
};

extern Global* global;

/* Logger source:
https://stackoverflow.com/questions/33365154/overloading-cout-for-logging-purposes
*/
class Logger {
public:
	Logger(std::string const& filename);
	Logger& operator<<(std::string const& str);
	Logger& operator<<(float const& fl);
private:
	std::ofstream stream_;
};

extern Logger logger;

class SeededRng {
public:

	//Inverse normal CDF Function - by John D. Cook (?)
	//Source code from https://www.johndcook.com/blog/normal_cdf_inverse/
	static double RationalApproximation(double t);
	static double norminv(double p); // p should be in between 0.0 to 1.0
	//End of Inverse Normal CDF Function

	const ulint MAXINTLOW = ulint(uint(-1)) + 1;
	const ulint MAX = ulint(18446744073709551615U);
	const ulint MUL = ulint(4495137515550960279U);
	const ulint ADD = ulint(13672451780744234491U);
	uint baseSeed;
	uint callSeed;

	SeededRng(uint initSeed);
	uint get();
	uint next();
	float next(float min, float max);
	float normal(float mean, float deviation);
};

extern uint boxVaoId;
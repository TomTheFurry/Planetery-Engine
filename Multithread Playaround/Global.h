#pragma once
#include <fstream>

#include <glm/glm.hpp>


#include "Definer.h"

#include "Camera.h"
#include "RenderProgram.h"




void printMatrix(mat4 mat);
void printVec3(vec3 vec);
void printQuat(quat q);
float gaussianFunction(float offset, float mean, float deviation);

class Global {
public:
	ivec2 windowSize = ivec2(1080,760);
	float fov = 60.0f;
	float windowRatio();
	int targetTps = 60;
	CameraMovable* activeCamera = nullptr;
	RenderProgram* r = nullptr;
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
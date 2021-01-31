
#define _USE_MATH_DEFINES
#include <math.h>
//For some reason, needs to be on top??????

#ifndef _MATH_DEFINES_DEFINED
#error Math Constant not defined??
#endif // !_MATH_DEFINES_DEFINED

#include <iostream>
#include <string>

#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <glfw/glfw3.h>




#include "Global.h"

float Global::windowRatio() {
	return (float)windowSize.x / (float)windowSize.y;
}

Global* global = new Global();


Logger::Logger(std::string const& filename)
	: stream_(filename, std::ofstream::out | std::ofstream::app)
{
	if (!stream_) {
		throw;
	}
}

Logger& Logger::operator<<(std::string const& str) {
	std::cout << str;
	stream_ << str;

	return *this;
}

Logger& Logger::operator<<(float const& str) {
	std::cout << str;
	stream_ << str;

	return *this;
}

Logger logger = Logger("Log.txt");

void printMatrix(glm::mat4 mat) {
	float* ptr = glm::value_ptr(mat);
	for (int i = 0; i < 16; i++) {
		logger << "[" << std::to_string(*(ptr + i)) << ", ";
		i++;
		logger << std::to_string(*(ptr + i)) << ", ";
		i++;
		logger << std::to_string(*(ptr + i)) << ", ";
		i++;
		logger << std::to_string(*(ptr + i)) << "]\n";

	}
	logger << "\n";
}

void printVec3(glm::vec3 vec) {
	logger << vec.x << ", " << vec.y << ", " << vec.z << "\n";
}

void printQuat(quat q)
{
	logger << q.x << ", " << q.y << ", " << q.z << ", " << q.w << "\n";
}

float gaussianFunction(float x, float m, float d) {
	return (1 / (d * sqrt(2 * M_PI))) * pow(M_E, -0.5 * pow((x - m) / d, 2));
}



//Inverse normal CDF Function - by John D. Cook (?)
//Source code from https://www.johndcook.com/blog/normal_cdf_inverse/
double SeededRng::RationalApproximation(double t) {
	double c[] = { 2.515517, 0.802853, 0.010328 };
	double d[] = { 1.432788, 0.189269, 0.001308 };
	return t - ((c[2] * t + c[1]) * t + c[0]) /
		(((d[2] * t + d[1]) * t + d[0]) * t + 1.0);
}

double SeededRng::norminv(double p) {
	if (p < 0.5) {
		return -RationalApproximation(sqrt(-2.0 * log(p)));
	}
	else {
		return RationalApproximation(sqrt(-2.0 * log(1 - p)));
	}
}
//End of Inverse Normal CDF Function

SeededRng::SeededRng(uint initSeed) {
	baseSeed = initSeed;
	callSeed = 0;
}

uint SeededRng::get() {
	return uint((callSeed ^ baseSeed) * MUL + ADD);
}

uint SeededRng::next() {
	callSeed = uint((callSeed ^ baseSeed) * MUL + ADD);
	return callSeed;
}

float SeededRng::next(float min, float max) {
	return float(double(next()) / double(MAXINTLOW) * double(max) - double(min)) + min;
}

float SeededRng::normal(float mean, float deviation) {
	return float(mean + deviation * norminv(double(next()) / double(MAXINTLOW)));
}

uint boxVaoId = uint(-1);

#include <iostream>
#include <string>
#include <glm/gtc/type_ptr.hpp>

#include "utility.h"

using uint = unsigned int;
using ulint = unsigned long long;



/*-----------Debug Function------------*/
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



/*----------Math Function------------*/

void bv2rgb(double& r, double& g, double& b, double bv) // RGB <0,1> <- BV <-0.4,+2.0> [-]
{
	double t;  r = 0.0; g = 0.0; b = 0.0; if (bv < -0.4) bv = -0.4; if (bv > 2.0) bv = 2.0;
	if ((bv >= -0.40) && (bv < 0.00)) { t = (bv + 0.40) / (0.00 + 0.40); r = 0.61 + (0.11 * t) + (0.1 * t * t); }
	else if ((bv >= 0.00) && (bv < 0.40)) { t = (bv - 0.00) / (0.40 - 0.00); r = 0.83 + (0.17 * t); }
	else if ((bv >= 0.40) && (bv < 2.10)) { t = (bv - 0.40) / (2.10 - 0.40); r = 1.00; }
	if ((bv >= -0.40) && (bv < 0.00)) { t = (bv + 0.40) / (0.00 + 0.40); g = 0.70 + (0.07 * t) + (0.1 * t * t); }
	else if ((bv >= 0.00) && (bv < 0.40)) { t = (bv - 0.00) / (0.40 - 0.00); g = 0.87 + (0.11 * t); }
	else if ((bv >= 0.40) && (bv < 1.60)) { t = (bv - 0.40) / (1.60 - 0.40); g = 0.98 - (0.16 * t); }
	else if ((bv >= 1.60) && (bv < 2.00)) { t = (bv - 1.60) / (2.00 - 1.60); g = 0.82 - (0.5 * t * t); }
	if ((bv >= -0.40) && (bv < 0.40)) { t = (bv + 0.40) / (0.40 + 0.40); b = 1.00; }
	else if ((bv >= 0.40) && (bv < 1.50)) { t = (bv - 0.40) / (1.50 - 0.40); b = 1.00 - (0.47 * t) + (0.1 * t * t); }
	else if ((bv >= 1.50) && (bv < 1.94)) { t = (bv - 1.50) / (1.94 - 1.50); b = 0.63 - (0.6 * t * t); }
}



/*---------Math Class---------------*/

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

bool pGreater::operator() (pQueueEntry a, pQueueEntry b) {
	return a.order > b.order;
}
bool pSmaller::operator() (pQueueEntry a, pQueueEntry b) {
	return a.order < b.order;
}

Logger logger = Logger("Log.txt");

pQueueEntry::pQueueEntry(double o, void* p)
{
	order = o;
	pointer = p;
}

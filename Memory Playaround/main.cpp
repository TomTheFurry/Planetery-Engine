#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include "MemoryBlock.h"

using ulint = unsigned long long int;
using uint = unsigned int;

template <typename T>
std::string nanoSec(T ns) {
	if (ns < 1000)			return std::to_string(ns) + "ns";
	if (ns < 1000000)		return std::to_string(ns / 1000.) + "us";
	if (ns < 1000000000)	return std::to_string(ns / 1000000.) + "ms";
	return std::to_string(ns / 1000000000.) + "s";
}

template <typename T>
std::string sec(T s) {
	return nanoSec(s * 1000000000);
}

const uint testCount = 0;

const ulint length = (ulint(1) << 32);

__declspec(noinline) void loggerS() {
	std::cout << "WARNING! NOT ZERO!!!\n";
}

//ulint ran[length];

void testMem() {
	MemoryBlockA<4> data{};
	for (ulint i = 0; i < length; i++) {
		uint& d = *(uint*)data.extend();
		d = i;
	}
	uint dump = 0;
	for (ulint i = 0; i < length; i++) {
		uint& d = *(uint*)data[i];
		dump += d;
	}
	for (ulint i = length; i > 0; i--) {
		uint& d = *(uint*)data[i-1];
		dump -= d;
	}
	if (dump != 0) loggerS();

	for (ulint i = 0; i < length; i++) {
		uint& d = *(uint*)data[i];
		d += i;
		//std::cout << d << "\n";
	}
}
void testMemB() {
	MemoryBlockB<4> data{};
	for (ulint i = 0; i < length; i++) {
		uint& d = *(uint*)data.extend();
		d = i;
	}
	uint dump = 0;
	for (ulint i = 0; i < length; i++) {
		uint& d = *(uint*)data[i];
		dump += d;
	}
	for (ulint i = length; i > 0; i--) {
		uint& d = *(uint*)data[i - 1];
		dump -= d;
	}
	if (dump != 0) loggerS();
	volatile uint pad = dump;
	for (ulint i = 0; i < length; i++) {
		uint& d = *(uint*)data[i];
		d += i;
	}
}
void testMemC() {
	MemoryBlockC<4> data{};
	for (ulint i = 0; i < length; i++) {
		uint& d = *(uint*)data.extend();
		d = i;
	}
	uint dump = 0;
	for (ulint i = 0; i < length; i++) {
		uint& d = *(uint*)data[i];
		dump += d;
	}
	for (ulint i = length; i > 0; i--) {
		uint& d = *(uint*)data[i-1];
		dump -= d;
	}
	if (dump != 0) loggerS();
	volatile uint pad = dump;
	for (ulint i = 0; i < length; i++) {
		uint& d = *(uint*)data[i];
		d += i;
	}
}
void testMemD() {
	MemoryBlockD<4> data{};
	for (ulint i = 0; i < length; i++) {
		uint& d = *(uint*)data.extend();
		d = i;
	}
	uint dump = 0;
	for (ulint i = 0; i < length; i++) {
		uint& d = *(uint*)data[i];
		dump += d;
	}
	for (ulint i = length; i > 0; i--) {
		uint& d = *(uint*)data[i - 1];
		dump -= d;
	}
	if (dump != 0) loggerS();
	volatile uint pad = dump;
	for (ulint i = 0; i < length; i++) {
		uint& d = *(uint*)data[i];
		d += i;
	}
}
void testVect() {
	std::vector<uint> data{};
	for (ulint i = 0; i < length; i++) {
		data.push_back(i);
	}
	uint dump = 0;
	for (ulint i = 0; i < length; i++) {
		dump += data[i];
	}
	for (ulint i = length; i > 0; i--) {
		dump -= data[i-1];
	}
	if (dump != 0) loggerS();
	volatile uint pad = dump;
	for (ulint i = 0; i < length; i++) {
		data[i] += i;
	}
}



void testMemDS() {
	MemoryBlockD<4> data{};

	//Write New
	auto start = std::chrono::high_resolution_clock::now();
	for (ulint i = 0; i < length; i++) {
		uint& d = *(uint*)data.extend();
		d = i;
	}
	auto end = std::chrono::high_resolution_clock::now();
	std::cout << "MemD Write New Done in " << nanoSec((end - start).count()) << "\n";

	uint dump = 0;

	//Read Seq
	start = std::chrono::high_resolution_clock::now();
	for (ulint i = 0; i < length; i++) {
		uint& d = *(uint*)data[i];
		dump += d;
	}
	end = std::chrono::high_resolution_clock::now();
	std::cout << "MemD Read Seq Done in " << nanoSec((end - start).count()) << "\n";

	//Read BackSeq
	start = std::chrono::high_resolution_clock::now();
	for (ulint i = length; i > 0; i--) {
		uint& d = *(uint*)data[i - 1];
		dump -= d;
	}
	end = std::chrono::high_resolution_clock::now();
	std::cout << "MemD Read BackSeq Done in " << nanoSec((end - start).count()) << "\n";

	if (dump != 0) loggerS();

	//AddOne
	start = std::chrono::high_resolution_clock::now();
	for (ulint i = 0; i < length; i++) {
		uint& d = *(uint*)data[i];
		d += i;
	}
	end = std::chrono::high_resolution_clock::now();
	std::cout << "MemD Read AddOne Done in " << nanoSec((end - start).count()) << "\n";
}

void testVectS() {
	std::vector<uint> data{};

	//Write New
	auto start = std::chrono::high_resolution_clock::now();
	for (ulint i = 0; i < length; i++) {
		data.push_back(i);
	}
	auto end = std::chrono::high_resolution_clock::now();
	std::cout << "Vect Write New Done in " << nanoSec((end - start).count()) << "\n";

	uint dump = 0;

	//Read Seq
	start = std::chrono::high_resolution_clock::now();
	for (ulint i = 0; i < length; i++) {
		dump += data[i];
	}
	end = std::chrono::high_resolution_clock::now();
	std::cout << "Vect Read Seq Done in " << nanoSec((end - start).count()) << "\n";

	//Read BackSeq
	start = std::chrono::high_resolution_clock::now();
	for (ulint i = length; i > 0; i--) {
		dump -= data[i - 1];
	}
	end = std::chrono::high_resolution_clock::now();
	std::cout << "Vect Read BackSeq Done in " << nanoSec((end - start).count()) << "\n";

	if (dump != 0) loggerS();

	//AddOne
	start = std::chrono::high_resolution_clock::now();
	for (ulint i = 0; i < length; i++) {
		data[i] += i;
	}
	end = std::chrono::high_resolution_clock::now();
	std::cout << "Vect Read AddOne Done in " << nanoSec((end - start).count()) << "\n";
}

void stdTest() {
	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();

	if (length <= (ulint(1) << 17)) {
		auto start = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < 1; i++) {
			testMem();
		}
		auto end = std::chrono::high_resolution_clock::now();
		std::cout << "MemA Done in " << nanoSec((end - start).count()) << "\n";
	}
	else {
		std::cout << "MemA Does not support size greater then " << (ulint(1) << 17) << "\n";
	}

	start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 1; i++) {
		testMemB();
	}
	end = std::chrono::high_resolution_clock::now();
	std::cout << "MemB Done in " << nanoSec((end - start).count()) << "\n";

	start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 1; i++) {
		testMemC();
	}
	end = std::chrono::high_resolution_clock::now();
	std::cout << "MemC Done in " << nanoSec((end - start).count()) << "\n";

	start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 1; i++) {
		testMemD();
	}
	end = std::chrono::high_resolution_clock::now();
	std::cout << "MemD Done in " << nanoSec((end - start).count()) << "\n";

	start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < 1; i++) {
		testVect();
	}
	end = std::chrono::high_resolution_clock::now();
	std::cout << "Vect Done in " << nanoSec((end - start).count()) << "\n";

	std::cout << length << "\n";
}

void partTest() {
	std::cout << length << "\n";
	testMemDS();
	std::cout << "now buzy wait for 5 secs..." << "\n";
	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	ulint ns = 0;
	while (ns < 5000000000) {
		end = std::chrono::high_resolution_clock::now();
		ns = (end - start).count();
	}

	std::cout << "continuing...\n";

	testVectS();

}

void printFunc() {
	for (uint i = 0; i < 50; i++) {
		std::cout << i <<
			": " << slocatedBlockB(i) <<
			", " << fstartBlockB(i) <<
			", " << fastexp2B(i) << "\n";
	}
}

void initRan() {
	std::cout << "Initing rand array...\n";
	for (ulint i = 0; i < length; i++) {
		//ran[i] = rand() % length;
	}
	std::cout << "Rand array done\n";
}

int main() {
	initRan();

	partTest();
	//printFunc();
	//testMemDS();

	return 0;
}


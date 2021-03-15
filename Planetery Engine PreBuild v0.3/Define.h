#pragma once
#include <string>

#define USE_VULKAN
//#define USE_OPENGL

typedef unsigned int uint;
typedef unsigned long long int ulint;
typedef long long int lint;


// time unit convertion / to string
#define NS_PER_US	1000
#define NS_PER_US_F 1000.
#define NS_PER_MS	1000000
#define NS_PER_MS_F 1000000.
#define NS_PER_S	1000000000
#define NS_PER_S_F	1000000000.
#define NS_PER_M	60000000000
#define NS_PER_M_F	60000000000.
#define NS_PER_H	360000000000
#define NS_PER_H_F	360000000000.

template<typename T> std::string nanoSec(T ns) {
	if (ns < NS_PER_US) return std::to_string(ns) + "ns";
	if (ns < NS_PER_MS) return std::to_string(ns / NS_PER_US_F) + "us";
	if (ns < NS_PER_S) return std::to_string(ns / NS_PER_MS_F) + "ms";
	if (ns < NS_PER_M) return std::to_string(ns / NS_PER_S_F) + "s";
	if (ns < NS_PER_H) return std::to_string(ns / NS_PER_M_F) + "min";
	return std::to_string(ns / NS_PER_H_F) + "hour";
}

template<typename T> std::string sec(T s) { return nanoSec(s * NS_PER_S); }


// storage unit convertion / to string
#define B_PER_KB   1024
#define B_PER_KB_F 1024.
#define B_PER_MB   1048576
#define B_PER_MB_F 1048576.
#define B_PER_GB   1073741824
#define B_PER_GB_F 1073741824.
#define B_PER_TB   1099511627776
#define B_PER_TB_F 1099511627776.

template<typename T> std::string byte(T B) {
	if (B < B_PER_KB) return std::to_string(B) + "B";
	if (B < B_PER_MB) return std::to_string(B / B_PER_KB_F) + "KB";
	if (B < B_PER_GB) return std::to_string(B / B_PER_MB_F) + "MB";
	if (B < B_PER_TB) return std::to_string(B / B_PER_GB_F) + "GB";
	return std::to_string(B / B_PER_TB_F) + "TB";
}

#ifdef _DEBUG
constexpr bool IS_DEBUG_MODE = true;
#else
constexpr bool IS_DEBUG_MODE = false;
#endif
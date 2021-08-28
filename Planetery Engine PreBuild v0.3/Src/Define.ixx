module;
#include "Marco.h"
export module Define;
export import "GlmModule.h";
import std.core;
export {
	template<typename T> using Ref = std::reference_wrapper<T>;
	template<typename T>
	std::span<const T> asSpan(std::initializer_list<T> & l) {
		return std::span(l.begin(), l.end());
	}
	typedef unsigned short usint;
	typedef unsigned int uint;
	typedef unsigned long long int ulint;
	typedef long long int lint;
	typedef unsigned char char_u8;
	typedef char16_t char_u16;
	typedef char32_t char_cp;
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
	constexpr auto NS_PER_US = 1000;
	constexpr auto NS_PER_US_F = 1000.;
	constexpr auto NS_PER_MS = 1000000;
	constexpr auto NS_PER_MS_F = 1000000.;
	constexpr auto NS_PER_S = 1000000000;
	constexpr auto NS_PER_S_F = 1000000000.;
	constexpr auto NS_PER_M = 60000000000;
	constexpr auto NS_PER_M_F = 60000000000.;
	constexpr auto NS_PER_H = 360000000000;
	constexpr auto NS_PER_H_F = 360000000000.;
	constexpr auto B_PER_KB = 1024;
	constexpr auto B_PER_KB_F = 1024.;
	constexpr auto B_PER_MB = 1048576;
	constexpr auto B_PER_MB_F = 1048576.;
	constexpr auto B_PER_GB = 1073741824;
	constexpr auto B_PER_GB_F = 1073741824.;
	constexpr auto B_PER_TB = 1099511627776;
	constexpr auto B_PER_TB_F = 1099511627776.;
#ifdef _DEBUG
	constexpr bool IS_DEBUG_MODE = true;
#else
	constexpr bool IS_DEBUG_MODE = false;
#endif
#ifdef SAFETY_CHECK
	constexpr bool DO_SAFETY_CHECK = true;
#else
	constexpr bool DO_SAFETY_CHECK = false;
#endif
#ifdef USE_VULKAN
	constexpr bool USING_VULKAN = true;
#else
	constexpr bool USING_VULKAN = false;
#endif
#ifdef USE_OPENGL
	constexpr bool USING_OPENGL = true;
#else
	constexpr bool USING_OPENGL = false;
#endif
}

export std::string nanoSec(auto ns) {
	if (ns < NS_PER_US) return std::to_string(ns) + "ns";
	if (ns < NS_PER_MS) return std::to_string(ns / NS_PER_US_F) + "us";
	if (ns < NS_PER_S) return std::to_string(ns / NS_PER_MS_F) + "ms";
	if (ns < NS_PER_M) return std::to_string(ns / NS_PER_S_F) + "s";
	if (ns < NS_PER_H) return std::to_string(ns / NS_PER_M_F) + "min";
	return std::to_string(ns / NS_PER_H_F) + "hour";
}
export std::string sec(auto s) { return nanoSec(s * NS_PER_S); }
export std::string byte(auto B) {
	if (B < B_PER_KB) return std::to_string(B) + "B";
	if (B < B_PER_MB) return std::to_string(B / B_PER_KB_F) + "KB";
	if (B < B_PER_GB) return std::to_string(B / B_PER_MB_F) + "MB";
	if (B < B_PER_TB) return std::to_string(B / B_PER_GB_F) + "GB";
	return std::to_string(B / B_PER_TB_F) + "TB";
}

//Source: https://stackoverflow.com/questions/5100718/integer-to-hex-string-in-c
export template<typename I>
std::string address(I w, size_t hex_len = sizeof(I) << 1) {
	static const char* digits = "0123456789ABCDEF";
	std::string rc(hex_len, '0');
	for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4)
		rc[i] = digits[(((size_t)w) >> j) & 0x0f];
	return rc;
}
module;
#include <assert.h>
#include "ConsoleFormat.h"
module Util;
import std.core;
import std.threading;
import std.memory;
import Define;
import Logger;
#define STD_NEXTLINE_CHAR '\n'
namespace sync {
	std::pair<Promisee<bool>, Promisor<bool>> sync::newPromises() {
		auto* ptr = new std::atomic<bool>(false);
		return std::make_pair(Promisee<bool>(ptr), Promisor<bool>(ptr));
	}
}
namespace utf {
	std::pair<const char_u8*, char_cp> utf::getCodePoint(const char_u8* ptr) {
		if (*ptr < 0x80u)
			return {ptr + 1, (char_cp)(*ptr)};	// 1 code unit: ASCII
		if (*ptr < 0xC3u)
			return {ptr + 1, 0xFFFDu};	// Invalid section or overlong encodings
										// for 2 code units
		if (*(ptr + 1) < 0x80u || *(ptr + 1) >= 0xC0u)
			return {ptr + 1, 0xFFFDu};	// 2nd code unit check
		char_cp result = (*(ptr + 1)) % 64;
		if (*ptr < 0xE0u) {	 // 2 code units
			result = (char_cp((*ptr) % 32) << 6) + result;
			return {ptr + 2, result};
		}
		if (*(ptr + 2) < 0x80u || *(ptr + 2) >= 0xC0u)
			return {ptr + 2, 0xFFFDu};	// 3rd code unit check
		result = (result << 6) + (*(ptr + 2)) % 64;
		if (*ptr < 0xF0u) {	 // 3 code units
			result = (char_cp((*ptr) % 16) << 12) + result;
			if (result <= 0x800u)
				return {
				  ptr + 3, 0xFFFDu};  // overlong encodings for 3 code units
			if (result >= 0xD800u && result < 0xE000u)
				return {ptr + 3, 0xFFFDu};	// surrigate halves check
			return {ptr + 3, result};
		}
		if (*(ptr + 3) < 0x80u || *(ptr + 3) >= 0xC0u)
			return {ptr + 3, 0xFFFDu};	// 4th code unit check
		result = (result << 6) + *(ptr + 3) % 64;
		if (*(ptr) < 0xF4u) {
			result = (char_cp((*ptr) % 8) << 24) + result;
			if (result >= 0x10000u && result < 0x110000u)
				return {ptr + 4, result};  // not overlong encodings for 4 code
										   // units and out of range code
		}
		return {ptr + 4, 0xFFFDu};
	}

	std::pair<const char_u16*, char_cp> utf::getCodePoint(const char_u16* ptr) {
		// TODO: Implement utf-16 decoding
		return {ptr + 1, 0xFFFDu};
	}


	UTFIterator<char_u8> utf::beginOfUTF8(const std::string& str) {
		return UTFIterator((const char_u8*)str.c_str());
	}
	UTFIterator<char_u8> utf::endOfUTF8(const std::string& str) {
		assert(*(str.c_str() + str.size()) == '\0');
		return UTFIterator((const char_u8*)str.c_str() + str.size());
	}
	UTFIterator<char_u8> utf::beginOfUTF8(const char* charStr) {
		return UTFIterator((const char_u8*)charStr);
	}
	UTFIterator<char_u8> utf::endOfUTF8(const char* charStr) {
		assert(*(charStr + std::strlen(charStr)) == '\0');
		return UTFIterator((const char_u8*)charStr + std::strlen(charStr));
	}
}


// BUG: Unkown issue with "\n newline missing" warning with logger. Not sure
// where the bug is.

MultilineString::MultilineString(const MultilineString& ms): _data(ms._data) {}
MultilineString::MultilineString(MultilineString&& ms) noexcept:
  _data(std::move(ms._data)) {}
MultilineString::MultilineString(): _data() {}
MultilineString::MultilineString(const std::string& str, size_t lineLimit):
  _data() {
	size_t pos = 0;
	size_t newPos;
	while ((newPos = str.find(STD_NEXTLINE_CHAR, pos)) != std::string::npos) {
		size_t strLen = newPos - pos;
		pos = newPos;
		for (; strLen > lineLimit; strLen -= lineLimit) {
			_data.emplace_back(str.substr(pos - strLen, lineLimit));
		}
		_data.emplace_back(str.substr(pos - strLen, strLen));
		pos++;
	}
	if (pos != str.length()) {
		_data.emplace_back(str.substr(pos) + format({COLOR_RED}) + "\\n"
						   + format({COLOR_DEFAULT}));
	}
}
MultilineString::MultilineString(const std::vector<std::string>& strlist):
  _data(strlist) {}
MultilineString::MultilineString(std::vector<std::string>&& strlist):
  _data(std::move(strlist)) {}
MultilineString::operator std::string() const {
	std::string a{};
	for (const auto& line : _data) {
		a.append(line);
		a.push_back(STD_NEXTLINE_CHAR);
	}
	return a;
}
MultilineString& MultilineString::operator<<(MultilineString&& join) {
	_data.insert(_data.end(), std::make_move_iterator(join._data.begin()),
	  std::make_move_iterator(join._data.end()));
	return *this;
}
std::string& MultilineString::getLine(size_t l) { return _data[l]; }
const std::string& MultilineString::getLine(size_t l) const { return _data[l]; }
size_t MultilineString::countLines() const { return _data.size(); }
void MultilineString::padLeft(const std::string& str) {
	for (auto& line : _data) { line.insert(0, str); }
}
void MultilineString::padRight(const std::string& str) {
	for (auto& line : _data) { line.append(str); }
}
std::ostream& operator<<(std::ostream& out, const MultilineString& str) {
	for (auto& line : str._data) { out << line << STD_NEXTLINE_CHAR; }
	return out;
}


util::Timer::Timer() {
	_time = 0;
	_start = std::chrono::high_resolution_clock::now();
}

ulint util::Timer::time() {
	return _time == 0
		   ? _time =
			   (std::chrono::high_resolution_clock::now() - _start).count()
		   : _time;
}

//Source: http://beesbuzz.biz/code/16-hsv-color-transforms
glm::vec4 util::transformHSV(glm::vec4 in,	// color to transform
  float h,							 // hue shift (in degrees)
  float s,							 // saturation multiplier (scalar)
  float v							 // value multiplier (scalar)
) {
	float vsu = v * s * cos(h * std::numbers::pi / 180);
	float vsw = v * s * sin(h * std::numbers::pi / 180);
	glm::vec4 ret{};
	ret.r = (.299 * v + .701 * vsu + .168 * vsw) * in.r
		  + (.587 * v - .587 * vsu + .330 * vsw) * in.g
		  + (.114 * v - .114 * vsu - .497 * vsw) * in.b;
	ret.g = (.299 * v - .299 * vsu - .328 * vsw) * in.r
		  + (.587 * v + .413 * vsu + .035 * vsw) * in.g
		  + (.114 * v - .114 * vsu + .292 * vsw) * in.b;
	ret.b = (.299 * v - .300 * vsu + 1.25 * vsw) * in.r
		  + (.587 * v - .588 * vsu - 1.05 * vsw) * in.g
		  + (.114 * v + .886 * vsu - .203 * vsw) * in.b;
	ret.a = in.a;
	return ret;
}
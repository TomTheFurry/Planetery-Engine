#include "UTF.h"
#include <iterator>
#include "Logger.h"
#include <assert.h>
using namespace utf;

std::pair<const char_u8*, char_cp> utf::getCodePoint(const char_u8* ptr) {
	if (*ptr < 0x80u) return {ptr+1, (char_cp)(*ptr)}; //1 code unit: ASCII
	if (*ptr < 0xC3u) return {ptr+1, 0xFFFDu}; //Invalid section or overlong encodings for 2 code units
	if (*(ptr+1)<0x80u||*(ptr+1)>=0xC0u) return {ptr+1, 0xFFFDu}; //2nd code unit check
	char_cp result = (*(ptr+1))%64;
	if (*ptr < 0xE0u) { // 2 code units
		result = (char_cp((*ptr)%32)<<6) + result;
		return {ptr+2, result};
	}
	if (*(ptr+2)<0x80u||*(ptr+2)>=0xC0u) return {ptr+2, 0xFFFDu}; //3rd code unit check
	result = (result<<6) + (*(ptr+2))%64;
	if (*ptr < 0xF0u) { //3 code units
		result = (char_cp((*ptr)%16)<<12) + result;
		if (result<=0x800u) return {ptr+3, 0xFFFDu}; //overlong encodings for 3 code units
		if (result>=0xD800u && result<0xE000u) return {ptr+3, 0xFFFDu}; //surrigate halves check
		return {ptr+3, result};
	}
	if (*(ptr+3)<0x80u||*(ptr+3)>=0xC0u) return {ptr+3, 0xFFFDu}; //4th code unit check
	result = (result<<6) + *(ptr+3)%64;
	if (*(ptr)<0xF4u) {
		result = (char_cp((*ptr)%8)<<24) + result;
		if (result >= 0x10000u && result < 0x110000u)
			return {ptr+4, result}; //not overlong encodings for 4 code units and out of range code
	}
	return {ptr+4, 0xFFFDu};
}

std::pair<const char_u16*, char_cp> utf::getCodePoint(const char_u16* ptr) {
	//TODO: Implement utf-16 decoding
	return {ptr+1, 0xFFFDu};
}


UTFIterator<char_u8> utf::beginOfUTF8(const std::string& str) {
	return UTFIterator((const char_u8*)str.c_str());
}
UTFIterator<char_u8> utf::endOfUTF8(const std::string& str) {
	assert(*(str.c_str()+str.size())=='\0');
	return UTFIterator((const char_u8*)str.c_str()+str.size());
}
UTFIterator<char_u8> utf::beginOfUTF8(const char* charStr) {
	return UTFIterator((const char_u8*)charStr);
}
UTFIterator<char_u8> utf::endOfUTF8(const char* charStr) {
	assert(*(charStr+std::strlen(charStr))=='\0');
	return UTFIterator((const char_u8*)charStr+std::strlen(charStr));
}
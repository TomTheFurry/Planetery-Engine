module;

#include <utility>
#include <assert.h>
#include <iterator>
#include <string>

export module UTF;
import Def;

typedef unsigned char char_u8;
typedef char16_t char_u16;
typedef char32_t char_cp;
export namespace utf {
	//if invalid returns CodePoint:uFFFD, Size:+1(1-Byte)
	extern std::pair<const char_u8*, char_cp> getCodePoint(const char_u8* ptr);
	//if invalid returns CodePoint:uFFFD, Size:+1(2-Byte)
	extern std::pair<const char_u16*, char_cp> getCodePoint(const char_u16* ptr);

	template <typename T>
	concept UTFEvenCharType = std::same_as<T, char_cp>;

	template <typename T>
	concept UTFUnevenCharType = std::same_as<T, char_u8> || std::same_as<T, char_u16>;

	template <typename T>
	concept UTFCharType = UTFUnevenCharType<T> || UTFEvenCharType<T>;
	
	//NOTE: WARN: Current method will read at end of iterator location.
	//However, it is currently not causing issue because char* should be
	//NULL terminated. This make the iterator treat it as 1 byte code point,
	//which will then not read after the NULL terrminator area and causes
	//Sec Fault. Though on string that is not NULL terminated, it could causes
	//terrifying issues!!!!
	template <UTFUnevenCharType C>
	class UTFIterator {
	public:
		using iterator_category = std::forward_iterator_tag;
		using difference_type = ptrdiff_t; //should be diff of size_t, but...
		using pointer_type = const C*;
		using value_type = char_cp;
		using reference = const char_cp&;
		UTFIterator() = default;
		UTFIterator(const C* ptr) : _ptr(ptr) {
			static_assert(std::forward_iterator<UTFIterator>);
			auto p = getCodePoint(ptr);
			_nextUnit = p.first;
			_currentCodePoint = p.second;
		};
		UTFIterator& operator++() {
			_ptr = _nextUnit;
			auto p = getCodePoint(_ptr);
			_nextUnit = p.first;
			_currentCodePoint = p.second;
			return *this;
		}
		UTFIterator operator++(int) {
			auto copy = UTFIterator(*this);
			++(*this);
			return copy;
		}
		const char_cp& operator*() const { return _currentCodePoint; }
		const char_cp* operator->() const { return &_currentCodePoint; }
		const C* operator&() const { return _ptr; }
		bool operator==(const UTFIterator& rhs) const {
			return (_ptr<rhs._nextUnit && rhs._ptr<_nextUnit);
		}
		std::weak_ordering operator<=>(const UTFIterator& rhs) const {
			if (*this == rhs) return std::weak_ordering::equivalent;
			return _ptr<=>rhs._ptr;
		}
	private:
		char_cp _currentCodePoint = 0xFFFDu;
		const C* _ptr = nullptr;
		const C* _nextUnit = nullptr;
	};

	extern UTFIterator<char_u8> beginOfUTF8(const std::string& str);
	extern UTFIterator<char_u8> endOfUTF8(const std::string& str);
	extern UTFIterator<char_u8> beginOfUTF8(const char* charStr);
	extern UTFIterator<char_u8> endOfUTF8(const char* charStr);
}

module :private;
import Logger;

using namespace utf;

std::pair<const char_u8*, char_cp> utf::getCodePoint(const char_u8* ptr) {
	if (*ptr < 0x80u) return { ptr + 1, (char_cp)(*ptr) }; //1 code unit: ASCII
	if (*ptr < 0xC3u) return { ptr + 1, 0xFFFDu }; //Invalid section or overlong encodings for 2 code units
	if (*(ptr + 1) < 0x80u || *(ptr + 1) >= 0xC0u) return { ptr + 1, 0xFFFDu }; //2nd code unit check
	char_cp result = (*(ptr + 1)) % 64;
	if (*ptr < 0xE0u) { // 2 code units
		result = (char_cp((*ptr) % 32) << 6) + result;
		return { ptr + 2, result };
	}
	if (*(ptr + 2) < 0x80u || *(ptr + 2) >= 0xC0u) return { ptr + 2, 0xFFFDu }; //3rd code unit check
	result = (result << 6) + (*(ptr + 2)) % 64;
	if (*ptr < 0xF0u) { //3 code units
		result = (char_cp((*ptr) % 16) << 12) + result;
		if (result <= 0x800u) return { ptr + 3, 0xFFFDu }; //overlong encodings for 3 code units
		if (result >= 0xD800u && result < 0xE000u) return { ptr + 3, 0xFFFDu }; //surrigate halves check
		return { ptr + 3, result };
	}
	if (*(ptr + 3) < 0x80u || *(ptr + 3) >= 0xC0u) return { ptr + 3, 0xFFFDu }; //4th code unit check
	result = (result << 6) + *(ptr + 3) % 64;
	if (*(ptr) < 0xF4u) {
		result = (char_cp((*ptr) % 8) << 24) + result;
		if (result >= 0x10000u && result < 0x110000u)
			return { ptr + 4, result }; //not overlong encodings for 4 code units and out of range code
	}
	return { ptr + 4, 0xFFFDu };
}

std::pair<const char_u16*, char_cp> utf::getCodePoint(const char_u16* ptr) {
	//TODO: Implement utf-16 decoding
	return { ptr + 1, 0xFFFDu };
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
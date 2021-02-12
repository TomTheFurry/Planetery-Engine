#pragma once
#include "Define.h"
#include <vector>

typedef unsigned char char_u8;
typedef char16_t char_u16;
typedef char32_t char_cp;

namespace utf {
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
}
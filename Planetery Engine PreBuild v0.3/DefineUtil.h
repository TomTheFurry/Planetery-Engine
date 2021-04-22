#pragma once

#include <concepts>
#include <vector>
#include <list>
#include <chrono>
#include <memory_resource>
#include <set>
#include <initializer_list>
#include "Define.h"

#ifdef SAFETY_CHECK
#	include <typeinfo>
#	include "Logger.h"
#endif

template<typename T>
concept Iterator = std::input_or_output_iterator<T>;

template<class ContainerType>
concept Container = requires(ContainerType a, const ContainerType b) {
	requires std::regular<ContainerType>;
	requires std::swappable<ContainerType>;
	requires std::destructible<typename ContainerType::value_type>;
	requires std::same_as<typename ContainerType::reference,
	  typename ContainerType::value_type&>;
	requires std::same_as<typename ContainerType::const_reference,
	  const typename ContainerType::value_type&>;
	requires std::forward_iterator<typename ContainerType::iterator>;
	requires std::forward_iterator<typename ContainerType::const_iterator>;
	requires std::signed_integral<typename ContainerType::difference_type>;
	requires std::same_as<typename ContainerType::difference_type,
	  typename std::iterator_traits<
		typename ContainerType::iterator>::difference_type>;
	requires std::same_as<typename ContainerType::difference_type,
	  typename std::iterator_traits<
		typename ContainerType::const_iterator>::difference_type>;
	{ a.begin() } -> std::same_as<typename ContainerType::iterator>;
	{ a.end() } -> std::same_as<typename ContainerType::iterator>;
	{ b.begin() } -> std::same_as<typename ContainerType::const_iterator>;
	{ b.end() } -> std::same_as<typename ContainerType::const_iterator>;
	{ a.cbegin() } -> std::same_as<typename ContainerType::const_iterator>;
	{ a.cend() } -> std::same_as<typename ContainerType::const_iterator>;
	{ a.size() } -> std::same_as<typename ContainerType::size_type>;
	{ a.max_size() } -> std::same_as<typename ContainerType::size_type>;
	{ a.empty() } -> std::same_as<bool>;
};

static_assert(Container<std::vector<uint>>);
static_assert(!Container<uint>);

template<Iterator Iter> class View
{
	Iter _begin;
	Iter _end;
  public:
	View(Iter begin, Iter end) {
		_begin = begin;
		_end = end;
	}
	View(Container auto& c) {
		_begin = c.begin();
		_end = c.end();
	}
	View(const Container auto& c) {
		_begin = c.cbegin();
		_end = c.cend();
	}
	Iter begin() { return _begin; }
	Iter end() { return _end; }
};
template<typename T, typename V>
concept Viewable = requires(V v) {
	{ v.begin() } -> std::forward_iterator;
	{ v.end() } -> Iterator;
	requires std::sentinel_for<decltype(v.end()), decltype(v.begin())>;
	requires std::convertible_to<decltype(*v.begin()), T>;

};


// typesafe flag decil
template<typename FlagType> class Flags
{
	using value = std::underlying_type<FlagType>::type;
	value _v;
  public:
	Flags() { _v = 0; }
	Flags(FlagType f) { _v = static_cast<value>(f); }
	Flags(value f) { _v = f; }
	explicit operator value() const { return _v; }
	operator bool() const { return _v != 0; }
	Flags operator|(Flags b) const { return _v | b._v; }
	Flags operator|(FlagType f) const { return _v | static_cast<value>(f); }
	Flags operator&(Flags b) const { return _v & b._v; }
	Flags operator&(FlagType f) const { return _v & static_cast<value>(f); }
	Flags operator^(Flags b) const { return _v ^ b._v; }
	Flags operator^(FlagType f) const { return _v ^ static_cast<value>(f); }

	Flags& operator|=(Flags b) { return (_v |= b._v, *this); }
	Flags& operator|=(FlagType f) {
		return (_v |= static_cast<value>(f), *this);
	}
	Flags& operator&=(Flags b) { return (_v &= b._v, *this); }
	Flags& operator&=(FlagType f) {
		return (_v &= static_cast<value>(f), *this);
	}
	Flags& operator^=(Flags b) { return (_v ^= b._v, *this); }
	Flags& operator^=(FlagType f) {
		return (_v ^= static_cast<value>(f), *this);
	}
	bool has(Flags b) const { return _v & b._v; }
	bool has(FlagType f) const { return _v & static_cast<value>(f); }
	Flags& set(Flags b) { return (_v |= b._v, *this); }
	Flags& set(FlagType f) { return (_v |= static_cast<value>(f), *this); }
	Flags& unset(Flags b) { return (_v |= ~b._v, *this); }
	Flags& unset(FlagType f) { return (_v |= ~static_cast<value>(f), *this); }
};
#define DECLARE_FLAG_TYPE(__type)                                            \
	Flags<__type> operator|(__type l, __type r) {                            \
		using value = std::underlying_type<__type>::type;                    \
		return Flags<__type>(static_cast<value>(l) | static_cast<value>(r)); \
	}                                                                        \
	Flags<__type> operator&(__type l, __type r) {                            \
		using value = std::underlying_type<__type>::type;                    \
		return Flags<__type>(static_cast<value>(l) & static_cast<value>(r)); \
	}                                                                        \
	Flags<__type> operator^(__type l, __type r) {                            \
		using value = std::underlying_type<__type>::type;                    \
		return Flags<__type>(static_cast<value>(l) ^ static_cast<value>(r)); \
	}                                                                        \
	Flags<__type> operator~(__type r) {                                      \
		using value = std::underlying_type<__type>::type;                    \
		return Flags<__type>(~static_cast<value>(r));                        \
	}


template<typename T> typename Flags<T>::value operator|(T a, T b) {
	return a | b;
}

template<typename T>
concept trivalType = std::is_trivial_v<T>;

namespace util {

	class Timer
	{
	  public:
		Timer();
		ulint time();  // In ns
	  private:
		std::chrono::steady_clock::time_point _start;
		ulint _time;
	};


	/// <summary>
	/// Booby Trap the templated class. Call trigger() to trigger the trap. Call
	/// isTriggered() to get the state. Call reset() to reset the trap.
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template<typename T> class Trap
	{
	  public:
		typedef T ValueType;
		template<typename... Args> Trap(Args&&... args):
		  _t(std::forward<Args>(args)...) {
			trigger();
		}
		void trigger() { _triggered = true; }
		bool isTriggered() const { return _triggered; }
		bool reset() { _triggered = false; }
		const T& operator*() const { return _t; }
		const T* operator->() const { return &_t; }
		T& operator*() {
			trigger();
			return _t;
		}
		T* operator->() {
			trigger();
			return &_t;
		}
	  private:
		T _t;
		bool _triggered;
	};
	template<typename P>
		requires std::is_pointer_v<P>
	class Trap<P>
	{
	  public:
		typedef P ValueType;
		template<typename... Args> Trap(Args&&... args):
		  _p(std::forward<Args>(args)...) {
			trigger();
		}
		void trigger() { _triggered = true; }
		bool isTriggered() const { return _triggered; }
		bool reset() { _triggered = false; }
		auto operator*() const { return *_p; }
		auto operator->() const { return _p; }
		auto operator*() { return *_p; }
		auto operator->() { return _p; }
	  private:
		P _p;
		bool _triggered;
	};

	template<std::semiregular T> class ValPtrUnion
	{
	  public:
		ValPtrUnion(T v);
		ValPtrUnion(const T& vPtr);
		operator const T&() const;
		operator const T*() const;
		ValPtrUnion& operator=(T v);
		ValPtrUnion& operator=(const T& vPtr);
	  protected:
		bool _isPtr;
		union {
			T _v;
			T* _vPtr;
		};
	};

	template<typename Resource,
	  typename Alloc = std::pmr::polymorphic_allocator<std::byte>>
	class PMRPair: public Alloc
	{
	  protected:
		Resource r;
	  public:
		PMRPair(auto... args):
		  r(std::forward<decltype(args)>(args)...), Alloc(&r) {}
	};

	class MBRPool: PMRPair<std::pmr::monotonic_buffer_resource>
	{
	  public:
		MBRPool(size_t initSize): PMRPair(initSize) {}
		MBRPool(const MBRPool&) = delete;
		template<trivalType T> T* make(std::initializer_list<T> t) {
			auto ptr = PMRPair::allocate_object<T>(t.size());
			std::copy(t.begin(), t.end(), ptr);
		}
		template<trivalType T> T* make(T t) {
			auto ptr = PMRPair::allocate_object<T>(1);
			*ptr = t;
			return ptr;
		}
		template<trivalType T> T* alloc(size_t n = 1) {
			return PMRPair::allocate_object<T>(n);
		}
	};
	template<typename T> class RepeatIterator
	{
	  public:
		using value_type = T;
		RepeatIterator() = default;
		RepeatIterator(const T& obj, size_t pos = 0) {
			static_assert(std::random_access_iterator<RepeatIterator>);
			_pos = pos;
			_ptr = &obj;
		}
		RepeatIterator(const T* ptr, size_t pos = 0) {
			_pos = pos;
			_ptr = ptr;
		};
		const T& operator*() const { return *_ptr; }
		const T* operator->() const { return _ptr; }
		const T& operator[](lint _) const { return *_ptr; }
		RepeatIterator& operator++() {
			_pos++;
			return *this;
		}
		RepeatIterator operator++(int) {
			auto copy = RepeatIterator(*this);
			++_pos;
			return copy;
		}
		RepeatIterator& operator--() {
			_pos--;
			return *this;
		}
		RepeatIterator operator--(int) {
			auto copy = RepeatIterator(*this);
			--_pos;
			return copy;
		}
		bool operator==(const RepeatIterator& rhs) const {
			return (_ptr == rhs._ptr && _pos == rhs._pos);
		}
		std::partial_ordering operator<=>(const RepeatIterator& rhs) const {
			if (_ptr != rhs._ptr) return std::partial_ordering::unordered;
			return _pos <=> rhs._pos;
		}
		lint operator-(const RepeatIterator& rhs) const {
			return _pos - rhs._pos;
		}
		RepeatIterator operator+(lint p) const {
			return RepeatIterator(_ptr, _pos + p);
		}
		RepeatIterator operator-(lint p) const {
			return RepeatIterator(_ptr, _pos - p);
		}
		RepeatIterator& operator+=(lint p) {
			_pos += p;
			return *this;
		}
		RepeatIterator& operator-=(lint p) {
			_pos -= p;
			return *this;
		}
		friend RepeatIterator operator+(lint lhs, const RepeatIterator& rhs) {
			return RepeatIterator(rhs._ptr, rhs._pos + lhs);
		}
		friend RepeatIterator operator-(lint lhs, const RepeatIterator& rhs) {
			return RepeatIterator(rhs._ptr, rhs._pos - lhs);
		}
	  private:
		const T* _ptr;
		size_t _pos;
	};

	auto makeRepeatedView(const auto& t, size_t n = 1) {
		return View(RepeatIterator(t), RepeatIterator(t, n));
	}
}

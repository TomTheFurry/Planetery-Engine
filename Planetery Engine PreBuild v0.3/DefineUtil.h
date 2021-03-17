#pragma once

#include <concepts>
#include <vector>
#include <list>
#include <chrono>

#include "Define.h"

// typesafe flag decil
template<typename T> class Flags
{};
#define DECLARE_FLAG_TYPE(__type)                         \
	template<> class Flags<__type>                        \
	{                                                     \
		using value = std::underlying_type<__type>::type; \
		value _v;                                         \
	  public:                                             \
		Flags(__type f) { _v = static_cast<value>(f); }   \
		Flags(value f) { _v = f; }                        \
		explicit operator value() { return _v; }          \
		Flags operator|(Flags b) { return _v | b._v; }    \
		Flags operator&(Flags b) { return _v & b._v; }    \
		Flags operator^(Flags b) { return _v ^ b._v; }    \
	}
template<typename T> typename Flags<T>::value operator|(T a, T b) {
	return a | b;
}

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


}

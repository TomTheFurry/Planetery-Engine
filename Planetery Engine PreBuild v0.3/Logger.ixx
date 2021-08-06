module;
#include "ConsoleFormat.h"
export module Logger;
import std.core;

export {
	class Log;
	template<typename T, typename... Args>
	void _loopStream(Log & l, T t, Args... a);
	template<typename T> void _loopStream(Log & l, T t);

	class Log
	{
	  public:
		void newThread();
		void closeThread();
		void newThread(std::string&& str);
		void closeThread(std::string&& str);
		void newLayer();
		void closeLayer();
		void newMessage();
		void closeMessage();
		void newMessage(std::string&& str);
		void closeMessage(std::string&& str);
		template<typename T> std::ostream& operator<<(T t) {
			return (getStream() << t);
		}
		Log();
		template<typename... Args> void operator()(Args... a);
	  private:
		std::ostream& getStream();
	};
	inline std::string format(const std::initializer_list<char> list);

	template<typename T, typename... Args>
	void _loopStream(Log & l, T t, Args... a) {
		l << std::forward<T>(t);
		_loopStream(l, a...);
	}
	template<typename T> void _loopStream(Log & l, T t) {
		l << std::forward<T>(t);
	}
	template<typename... Args> void Log::operator()(Args... a) {
		this->newLayer();
		_loopStream(*this, a...);
		this->closeLayer();
	}

	inline std::string format(const std::initializer_list<char> list) {
		std::string str{FORMAT_START};
		str += std::to_string(int(*list.begin()));
		for (auto it = list.begin() + 1; it != list.end(); it++) {
			str += FORMAT_MID;
			str += std::to_string(int(*it));
		}
		return (str += FORMAT_END);
	}
	extern Log logger;
}
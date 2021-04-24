export module Util;
import std.core;
import std.threading;
import Define;

export namespace sync {
	template <typename T> class Promisee;
	template <typename T> class Promisor;
	template<typename T> std::pair<Promisee<T>, Promisor<T>> newPromisedData(T&& data);
	template<typename T>
	class Promisee {
		std::atomic<bool>* _c;
		std::atomic<T>* _d;
	protected:
		Promisee(std::atomic<bool>* c, std::atomic<T>* d) noexcept :
			_c(c), _d(d) {}
	public:
		Promisee(const Promisee& p) = delete;
		Promisee(Promisee&& p) noexcept :
			_c(p._c), _d(p._d) {
			p._c = nullptr;
		};
		const std::atomic<bool>* operator->() const noexcept {
			return _c;
		}
		//Will block if thread haven't gotten the acknowledgement yet.
		~Promisee() noexcept {
			if (_c != nullptr) {
				_c->wait(false, std::memory_order_relaxed);
				delete _c;
				delete _d;
			}
		};
		template<typename U>
		friend std::pair<Promisee<U>, Promisor<U>> newPromisedData<U>(U&& data);
		//friend std::pair<Promisee<T>, Promisor<T>> newPromisedData<T>(T&& data);
	};
	template <>
	class Promisee<bool> {
		std::atomic<bool>* _c;
	protected:
		Promisee(std::atomic<bool>* c) noexcept : _c(c) {}
	public:
		Promisee(const Promisee<bool>& p) = delete;
		Promisee(Promisee<bool>&& p) noexcept : _c(p._c) {
			p._c = nullptr;
		}
		const std::atomic<bool>* operator->() const noexcept {
			return _c;
		}
		//Will block if thread haven't gotten the acknowledgement yet.
		~Promisee() noexcept {
			if (_c != nullptr) {
				_c->wait(false, std::memory_order_relaxed);
				delete _c;
			}
		}
		friend std::pair<Promisee<bool>, Promisor<bool>> newPromises();
	};

	template<typename T>
	class Promisor {
		std::atomic<bool>* _c;
		std::atomic<T>* _d;
	protected:
		Promisor(std::atomic<bool>* c, std::atomic<T>* d) noexcept :
			_c(c), _d(d) {}
	public:
		Promisor(const Promisor<T>& p) = delete;
		Promisor(Promisor<T>&& p) noexcept : _c(p._c), _d(p._d) {
			p._c = nullptr;
		}
		void sendPromise() noexcept {
			_c->store(true, std::memory_order_relaxed);
			_c = nullptr;
		};
		//Will send promise if not done already.
		~Promisor() noexcept {
			if (_c != nullptr) {
				_c->store(true, std::memory_order_relaxed);
			}
		}
		template<typename U>
		friend std::pair<Promisee<U>, Promisor<U>> newPromisedData<U>(U&& data);
		//friend std::pair<Promisee<T>, Promisor<T>> newPromisedData<T>(T&& data);
	};
	template <>
	class Promisor<bool> {
		std::atomic<bool>* _c;
	protected:
		Promisor(std::atomic<bool>* c) noexcept : _c(c) {}
	public:
		Promisor(const Promisor& p) = delete;
		Promisor(Promisor&& p) noexcept : _c(p._c) {
			p._c = nullptr;
		}
		void sendPromise() noexcept {
			_c->store(true, std::memory_order_relaxed);
			_c = nullptr;
		}
		//Will send promise if not done already.
		~Promisor() noexcept {
			if (_c != nullptr) {
				_c->store(true, std::memory_order_relaxed);
			}
		}
		friend std::pair<Promisee<bool>, Promisor<bool>> newPromises();
	};
	std::pair<Promisee<bool>, Promisor<bool>> newPromises();

	template<typename T>
	std::pair<Promisee<T>, Promisor<T>> newPromisedData(T&& data) {
		auto* ptr = new std::atomic<bool>(false);
		auto* dPtr = new std::atomic<T>(std::forward<T>(data));
		return std::make_pair(Promisee<T>(ptr, dPtr), Promisor<T>(ptr, dPtr));
	}
}

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
		//using iterator_category = std::forward_iterator_tag;
		//using difference_type = ptrdiff_t; //should be diff of size_t, but...
		//using pointer_type = const C*;
		//using value_type = char_cp;
		//using reference = const char_cp&;
		UTFIterator() = default;
		UTFIterator(const C* ptr) : _ptr(ptr) {
			//static_assert(std::forward_iterator<UTFIterator>); //MODULE HOTFIX
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
			return (_ptr < rhs._nextUnit&& rhs._ptr < _nextUnit);
		}
		std::weak_ordering operator<=>(const UTFIterator& rhs) const {
			if (*this == rhs) return std::weak_ordering::equivalent;
			return _ptr <=> rhs._ptr;
		}
	private:
		char_cp _currentCodePoint = 0xFFFDu;
		const C* _ptr = nullptr;
		const C* _nextUnit = nullptr;
	};

	UTFIterator<char_u8> beginOfUTF8(const std::string& str);
	UTFIterator<char_u8> endOfUTF8(const std::string& str);
	UTFIterator<char_u8> beginOfUTF8(const char* charStr);
	UTFIterator<char_u8> endOfUTF8(const char* charStr);
}

export class MultiStream : public std::ostream {
	struct MultiBuffer : public std::streambuf {
		void addBuffer(std::streambuf* buf) {
			bufs.push_back(buf);
		}
		virtual int overflow(int c) {
			std::for_each(bufs.begin(), bufs.end(), std::bind(std::mem_fn(&std::streambuf::sputc), std::placeholders::_1, c));
			return c;
		}

	private:
		std::vector<std::streambuf*> bufs;

	};
	MultiBuffer myBuffer;
public:
	MultiStream() : std::ostream(&myBuffer) {}
	void linkStream(std::ostream& out) {
		out.flush();
		myBuffer.addBuffer(out.rdbuf());
	}
};

export class MultilineString {
public:
	MultilineString();
	MultilineString(const MultilineString& ms);
	MultilineString(MultilineString&& ms) noexcept;
	MultilineString(const std::string& str, size_t lineLimit = -1);
	MultilineString(const std::vector<std::string>& strlist);
	MultilineString(std::vector<std::string>&& strlist);
	operator std::string() const;
	MultilineString& operator<<(MultilineString&& join);
	std::string& getLine(size_t l);
	const std::string& getLine(size_t l) const;
	size_t countLines() const;
	void padLeft(const std::string& str);
	void padRight(const std::string& str);
	friend std::ostream& operator<<(std::ostream& out, const MultilineString& str);
protected:
	std::vector<std::string> _data;
};

export template <typename I, typename R, size_t N>
class RollingAverage {
public:
	unsigned int index;
	I runningTotal;
	I buffer[N];

	RollingAverage() {
		clear();
	}

	void clear() {
		runningTotal = 0;
		std::fill(buffer, buffer + N, 0);
		index = 0;
	}

	R next(I inputValue) {
		/*add new value*/
		runningTotal -= buffer[index];
		buffer[index] = inputValue;
		index++;
		index %= N;
		/*update running total*/
		runningTotal += inputValue;
		/*calculate average*/
		return static_cast<R>(runningTotal) / static_cast<R>(N);
	}

	R get() {
		return static_cast<R>(runningTotal) / static_cast<R>(N);
	}
};
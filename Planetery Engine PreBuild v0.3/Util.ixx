module;
export module Util;
import std.core;
import std.threading;
import std.memory;
import Define;

export namespace sync {
	template<typename T> class Promisee;
	template<typename T> class Promisor;
	template<typename T>
	std::pair<Promisee<T>, Promisor<T>> newPromisedData(T&& data);
	template<typename T> class Promisee
	{
		std::atomic<bool>* _c;
		std::atomic<T>* _d;

	  protected:
		Promisee(std::atomic<bool>* c, std::atomic<T>* d) noexcept:
		  _c(c), _d(d) {}

	  public:
		Promisee(const Promisee& p) = delete;
		Promisee(Promisee&& p) noexcept: _c(p._c), _d(p._d) { p._c = nullptr; };
		const std::atomic<bool>* operator->() const noexcept { return _c; }
		// Will block if thread haven't gotten the acknowledgement yet.
		~Promisee() noexcept {
			if (_c != nullptr) {
				_c->wait(false, std::memory_order_relaxed);
				delete _c;
				delete _d;
			}
		};
		template<typename U>
		friend std::pair<Promisee<U>, Promisor<U>> newPromisedData<U>(U&& data);
		// friend std::pair<Promisee<T>, Promisor<T>> newPromisedData<T>(T&&
		// data);
	};
	template<> class Promisee<bool>
	{
		std::atomic<bool>* _c;

	  protected:
		Promisee(std::atomic<bool>* c) noexcept: _c(c) {}

	  public:
		Promisee(const Promisee<bool>& p) = delete;
		Promisee(Promisee<bool>&& p) noexcept: _c(p._c) { p._c = nullptr; }
		const std::atomic<bool>* operator->() const noexcept { return _c; }
		// Will block if thread haven't gotten the acknowledgement yet.
		~Promisee() noexcept {
			if (_c != nullptr) {
				_c->wait(false, std::memory_order_relaxed);
				delete _c;
			}
		}
		friend std::pair<Promisee<bool>, Promisor<bool>> newPromises();
	};

	template<typename T> class Promisor
	{
		std::atomic<bool>* _c;
		std::atomic<T>* _d;

	  protected:
		Promisor(std::atomic<bool>* c, std::atomic<T>* d) noexcept:
		  _c(c), _d(d) {}

	  public:
		Promisor(const Promisor<T>& p) = delete;
		Promisor(Promisor<T>&& p) noexcept: _c(p._c), _d(p._d) {
			p._c = nullptr;
		}
		void sendPromise() noexcept {
			_c->store(true, std::memory_order_relaxed);
			_c = nullptr;
		};
		// Will send promise if not done already.
		~Promisor() noexcept {
			if (_c != nullptr) { _c->store(true, std::memory_order_relaxed); }
		}
		template<typename U>
		friend std::pair<Promisee<U>, Promisor<U>> newPromisedData<U>(U&& data);
		// friend std::pair<Promisee<T>, Promisor<T>> newPromisedData<T>(T&&
		// data);
	};
	template<> class Promisor<bool>
	{
		std::atomic<bool>* _c;

	  protected:
		Promisor(std::atomic<bool>* c) noexcept: _c(c) {}

	  public:
		Promisor(const Promisor& p) = delete;
		Promisor(Promisor&& p) noexcept: _c(p._c) { p._c = nullptr; }
		void sendPromise() noexcept {
			_c->store(true, std::memory_order_relaxed);
			_c = nullptr;
		}
		// Will send promise if not done already.
		~Promisor() noexcept {
			if (_c != nullptr) { _c->store(true, std::memory_order_relaxed); }
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
	// if invalid returns CodePoint:uFFFD, Size:+1(1-Byte)
	extern std::pair<const char_u8*, char_cp> getCodePoint(const char_u8* ptr);
	// if invalid returns CodePoint:uFFFD, Size:+1(2-Byte)
	extern std::pair<const char_u16*, char_cp> getCodePoint(
	  const char_u16* ptr);

	template<typename T> concept UTFEvenCharType = std::same_as<T, char_cp>;

	template<typename T> concept UTFUnevenCharType =
	  std::same_as<T, char_u8> || std::same_as<T, char_u16>;

	template<typename T> concept UTFCharType =
	  UTFUnevenCharType<T> || UTFEvenCharType<T>;

	// NOTE: WARN: Current method will read at end of iterator location.
	// However, it is currently not causing issue because char* should be
	// NULL terminated. This make the iterator treat it as 1 byte code point,
	// which will then not read after the NULL terrminator area and causes
	// Sec Fault. Though on string that is not NULL terminated, it could causes
	// terrifying issues!!!!
	template<UTFUnevenCharType C> class UTFIterator
	{
	  public:
		using iterator_category = std::forward_iterator_tag;
		using difference_type =
		  std::ptrdiff_t;  // should be diff of size_t, but...
		using pointer_type = const C*;
		using value_type = char_cp;
		using reference = const char_cp&;
		UTFIterator() = default;
		UTFIterator(const C* ptr): _ptr(ptr) {
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
			return (_ptr < rhs._nextUnit && rhs._ptr < _nextUnit);
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

export class MultiStream: public std::ostream
{
	struct MultiBuffer: public std::streambuf {
		void addBuffer(std::streambuf* buf) { bufs.push_back(buf); }
		virtual int overflow(int c) {
			std::for_each(bufs.begin(), bufs.end(),
			  std::bind(
				std::mem_fn(&std::streambuf::sputc), std::placeholders::_1, c));
			return c;
		}

	  private:
		std::vector<std::streambuf*> bufs;
	};
	MultiBuffer myBuffer;

  public:
	MultiStream(): std::ostream(&myBuffer) {}
	void linkStream(std::ostream& out) {
		out.flush();
		myBuffer.addBuffer(out.rdbuf());
	}
};

export class MultilineString
{
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
	friend std::ostream& operator<<(
	  std::ostream& out, const MultilineString& str);

  protected:
	std::vector<std::string> _data;
};

export template<typename I, typename R, size_t N> class RollingAverage
{
  public:
	unsigned int index;
	I runningTotal;
	I buffer[N];

	RollingAverage() { clear(); }

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

	R get() { return static_cast<R>(runningTotal) / static_cast<R>(N); }
};

export template<typename T> concept Iterator = std::input_or_output_iterator<T>;

export template<class ContainerType> concept Container =
  requires(ContainerType a, const ContainerType b) {
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
	{ a.begin() }
	->std::same_as<typename ContainerType::iterator>;
	{ a.end() }
	->std::same_as<typename ContainerType::iterator>;
	{ b.begin() }
	->std::same_as<typename ContainerType::const_iterator>;
	{ b.end() }
	->std::same_as<typename ContainerType::const_iterator>;
	{ a.cbegin() }
	->std::same_as<typename ContainerType::const_iterator>;
	{ a.cend() }
	->std::same_as<typename ContainerType::const_iterator>;
	{ a.size() }
	->std::same_as<typename ContainerType::size_type>;
	{ a.max_size() }
	->std::same_as<typename ContainerType::size_type>;
	{ a.empty() }
	->std::same_as<bool>;
};

static_assert(Container<std::vector<uint>>);
static_assert(!Container<uint>);

export template<Iterator Iter> class View
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
export template<typename T, typename V> concept Viewable = requires(V v) {
	{ v.begin() }
	->std::forward_iterator;
	{ v.end() }
	->Iterator;
	requires std::sentinel_for<decltype(v.end()), decltype(v.begin())>;
	requires std::convertible_to<decltype(*v.begin()), T>;
};

export template<std::integral FlagType> class Flags
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

export template<typename T> concept trivalType = std::is_trivial_v<T>;
export template<typename T> concept integral_based =
  std::integral<std::underlying_type<T>::type>;

export template<integral_based T> auto operator|(T l, T r) {
	using value = std::underlying_type<T>::type;
	return Flags<T>(static_cast<value>(l) | static_cast<value>(r));
}
export template<integral_based T> auto operator&(T l, T r) {
	using value = std::underlying_type<T>::type;
	return Flags<T>(static_cast<value>(l) & static_cast<value>(r));
}
export template<integral_based T> auto operator^(T l, T r) {
	using value = std::underlying_type<T>::type;
	return Flags<T>(static_cast<value>(l) ^ static_cast<value>(r));
}
export template<integral_based T> auto operator~(T r) {
	using value = std::underlying_type<T>::type;
	return Flags<T>(~static_cast<value>(r));
}


export namespace util {

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
	template<typename P> requires std::is_pointer_v<P> class Trap<P>
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

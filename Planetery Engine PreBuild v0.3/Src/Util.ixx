export module Util;
import "Assert.h";
import std.core;
import std.threading;
import std.memory;
import Define;

export typedef std::pmr::monotonic_buffer_resource MonotonicBufferResource;

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

	template<typename T>
	concept UTFEvenCharType = std::same_as<T, char_cp>;

	template<typename T>
	concept UTFUnevenCharType =
	  std::same_as<T, char_u8> || std::same_as<T, char_u16>;

	template<typename T>
	concept UTFCharType = UTFUnevenCharType<T> || UTFEvenCharType<T>;

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


//#define LIKELY [[likely]]
//#define UNLIKELY [[unlikely]]
#define LIKELY
#define UNLIKELY

constexpr size_t DEFAULT_ALIGNMEMT = alignof(std::max_align_t);
constexpr size_t DEFAULT_BLOCK_SIZE = 256;

constexpr size_t alignUpTo(size_t p, size_t a) {
	return (p >= a && p % a == 0) ? p : (p + a - p % a);
}

[[noreturn]] inline void DetectedCritcalMemoryCorruption() {
	perror("Unexpected Critcal Memory Corruption detected. Halting program...");
	abort();
}

export namespace pmr {
	// exceptions
	struct BadArgException {};

	struct BadAllocException {
		size_t requestedSize;
		size_t requestedAlignment;
	};
	struct BadDeallocException {
		void* requestedPtr;
		size_t requestedSize;
		size_t requestedAlignment;
	};
	struct CorruptedMemoryException {};

	using Byte = std::byte;
	template<typename T> using Allocator = std::pmr::polymorphic_allocator<T>;
	using MemoryResource = std::pmr::memory_resource;
	using ThreadUnsafePoolResource = std::pmr::unsynchronized_pool_resource;
	using ThreadSafePoolResource = std::pmr::synchronized_pool_resource;
	using MonotonicResource = std::pmr::monotonic_buffer_resource;

	template<size_t AlignSize = DEFAULT_ALIGNMEMT> class StackMemoryResource:
	  public MemoryResource
	{
		static_assert((AlignSize & (AlignSize - 1)) == 0 && AlignSize != 0,
		  "StackMemoryResource<>: AlignSize must be power of 2.");

	  public:
		StackMemoryResource();
		explicit StackMemoryResource(MemoryResource* upstream);
		explicit StackMemoryResource(size_t blockSize);
		StackMemoryResource(size_t blockSize, MemoryResource* upstream);
		StackMemoryResource(const StackMemoryResource&) = delete;

		virtual void* do_allocate(size_t bytes, size_t alignment) final;
		virtual void do_deallocate(
		  void* p, size_t bytes, size_t alignment) noexcept final;
		virtual bool do_is_equal(
		  const MemoryResource& other) const noexcept final;

		[[nodiscard]] void* allocate(
		  size_t bytes, size_t alignment = AlignSize) {
			return do_allocate(bytes, alignment);
		}

		void deallocate(
		  void* p, size_t bytes, size_t alignment = AlignSize) noexcept {
			do_deallocate(p, bytes, alignment);
		}

		bool is_equal(const memory_resource& other) const noexcept {
			do_is_equal(other);
		}

		virtual ~StackMemoryResource() noexcept final;

		void release();

		MemoryResource* upstreamResource() const;
		size_t blockSize() const;

	  private:
		MemoryResource* _upstream;
		Byte* _stackPtr;
		Byte* _currentBlockTop;
		size_t _blockSize;

		/* Memory Layout
		 *
		 *  XX XX XX XX XX XX XX XX
		 *  NN NN NN NN DD DD DD DD
		 *  DD DD -- -- DD DD DD --
		 *  DD -- -- -- DD DD DD DD
		 *  DD DD DD DD DD DD -- --
		 *  __ __ __ __ SS SS SS SS
		 *  XX XX XX XX XX XX XX XX
		 *
		 *  X: OutOfBound Memory
		 *  N: Pointer to previous block's end '... __ __|SS SS ...'
		 *  D: Data allocated from this resource
		 *  -: Wasted space due to alignment
		 *  _: Wasted space due to last alloc not fitting
		 *  S: Pointer to the end of this block's stack '... -- --|__ __ ...'
		 *  Note: All Data is aligned to alignof(size_t)
		 *
		 *  _currentBlockTop: Ptr to current blocks's end '... __ __|SS SS ...'
		 *  _stackPtr: Ptr to where next new alloc should be
		 *  _blockSize: Size of usable space 'NN NN[DD DD ......__ __]SS SS'
		 *
		 */
	};

	template<typename T, typename MR> class pmrAllocator:
	  public std::allocator<T>
	{
		MR* mr;

	  public:
		pmrAllocator(MR* memory_resource): mr(memory_resource) {
			auto ndr = std::pmr::new_delete_resource();
		}
		[[nodiscard]] constexpr T* allocate(size_t n) {
			return (T*)(mr->allocate(n));
		}
		constexpr void deallocate(T* p, size_t n) { mr->deallocate(p, n); }

		template<typename U> [[nodiscard]] U* allocate_object(size_t n) {
			return (U*)(mr->allocate(sizeof(U) * n));
		}
		template<typename U> void deallocate_object(U* p, size_t n) {
			mr->deallocate(p, sizeof(U) * n);
		}
	};
}




export template<typename T>
concept Iterator = std::input_or_output_iterator<T>;

export template<class ContainerType>
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

export template<typename C>
concept ConstContainer = std::is_const_v<C> && Container<std::remove_cvref<C>>;
export template<typename C>
concept NonConstContainer =
  (!std::is_const_v<C>)&&Container<std::remove_cvref<C>>;

static_assert(Container<std::vector<uint>>);
static_assert(!Container<uint>);

export template<typename V>
concept Viewable = requires(V v) {
	{ v.begin() } -> Iterator;
	{ v.end() } -> Iterator;
	requires std::sentinel_for<decltype(v.begin()), decltype(v.end())>;
};
export template<typename V, typename T>
concept ViewableWith = requires(V v) {
	requires Viewable<V>;
	requires std::convertible_to<decltype(*v.begin()), T>;
};

template<Iterator Iter> struct iterator_return {
	using type = decltype(*(std::declval<Iter>()));
};

export template<typename BaseIt, typename Func, Func f>
requires std::regular_invocable<Func, typename iterator_return<BaseIt>::type>
class ConvertedIterator: public BaseIt
{
  public:
	using base_return_type = typename iterator_return<BaseIt>::type;
	using value_type = std::invoke_result_t<Func, base_return_type>;
	ConvertedIterator(): BaseIt() {}
	ConvertedIterator(const ConvertedIterator&) = default;
	ConvertedIterator(BaseIt it): BaseIt(it) {}
	value_type operator*() const { return f(BaseIt::operator*()); }
	ConvertedIterator& operator++() {
		BaseIt::operator++();
		return *this;
	}
	ConvertedIterator operator++(int) {
		auto _b = *this;
		BaseIt::operator++(0);
		return _b;
	}
};

export template<typename BaseIt, typename Func, Func func>
requires std::invocable<Func, typename iterator_return<BaseIt>::type>
class CView
{
	typedef ConvertedIterator<BaseIt, decltype(func), func> Iter;
	Iter _begin;
	Iter _end;

  public:
	CView(BaseIt begin, BaseIt end): _begin(begin), _end(end) {}
	template<NonConstContainer C> CView(C c) requires std::is_reference_v<C> {
		_begin = c.begin();
		_end = c.end();
	}
	template<ConstContainer C> CView(C c) requires std::is_reference_v<C> {
		_begin = c.cbegin();
		_end = c.cend();
	}
	Iter begin() { return _begin; }
	Iter end() { return _end; }
	template<typename FuncInner, FuncInner fin> auto pipeWith() {
		return CView<Iter, FuncInner, fin>(_begin, _end);
	}
};

export template<typename BaseIt> class View
{
	BaseIt _begin;
	BaseIt _end;

  public:
	View(BaseIt begin, BaseIt end): _begin(begin), _end(end) {}
	template<NonConstContainer C> View(C c) requires std::is_reference_v<C> {
		_begin = c.begin();
		_end = c.end();
	}
	template<ConstContainer C> View(C c) requires std::is_reference_v<C> {
		_begin = c.cbegin();
		_end = c.cend();
	}
	BaseIt begin() { return _begin; }
	BaseIt end() { return _end; }
	template<typename Func, Func f> auto pipeWith() {
		return CView<BaseIt, Func, f>(_begin, _end);
	}
};

export template<typename T>
concept trivalType = std::is_trivial_v<T>;
export template<typename T>
concept integral_based = std::integral<typename std::underlying_type<T>::type>;

export template<integral_based FlagType> class Flags
{
	using value = std::underlying_type<FlagType>::type;
	value _v;

  public:
	constexpr Flags() { _v = 0; }
	constexpr Flags(FlagType f) { _v = static_cast<value>(f); }
	constexpr Flags(value f) { _v = f; }
	constexpr explicit operator value() const { return _v; }
	constexpr value toVal() const { return _v; }
	constexpr operator bool() const { return _v != 0; }
	constexpr Flags operator|(Flags b) const { return _v | b._v; }
	constexpr Flags operator|(FlagType f) const {
		return _v | static_cast<value>(f);
	}
	constexpr Flags operator&(Flags b) const { return _v & b._v; }
	constexpr Flags operator&(FlagType f) const {
		return _v & static_cast<value>(f);
	}
	constexpr Flags operator^(Flags b) const { return _v ^ b._v; }
	constexpr Flags operator^(FlagType f) const {
		return _v ^ static_cast<value>(f);
	}

	constexpr Flags& operator|=(Flags b) { return (_v |= b._v, *this); }
	constexpr Flags& operator|=(FlagType f) {
		return (_v |= static_cast<value>(f), *this);
	}
	constexpr Flags& operator&=(Flags b) { return (_v &= b._v, *this); }
	constexpr Flags& operator&=(FlagType f) {
		return (_v &= static_cast<value>(f), *this);
	}
	constexpr Flags& operator^=(Flags b) { return (_v ^= b._v, *this); }
	constexpr Flags& operator^=(FlagType f) {
		return (_v ^= static_cast<value>(f), *this);
	}
	constexpr bool operator==(Flags b) const { return b._v == _v; }
	constexpr bool operator==(FlagType f) const {
		return static_cast<value>(f) == _v;
	}

	constexpr bool has(Flags b) const { return _v & b._v; }
	constexpr bool has(FlagType f) const { return _v & static_cast<value>(f); }
	constexpr Flags& set(Flags b) { return (_v |= b._v, *this); }
	constexpr Flags& set(FlagType f) {
		return (_v |= static_cast<value>(f), *this);
	}
	constexpr Flags& unset(Flags b) { return (_v |= ~b._v, *this); }
	constexpr Flags& unset(FlagType f) {
		return (_v |= ~static_cast<value>(f), *this);
	}
	friend constexpr Flags operator|(FlagType l, FlagType r) {
		return Flags(static_cast<value>(l) | static_cast<value>(r));
	}
	friend constexpr Flags operator&(FlagType l, FlagType r) {
		return Flags(static_cast<value>(l) & static_cast<value>(r));
	}
	friend constexpr Flags operator^(FlagType l, FlagType r) {
		return Flags(static_cast<value>(l) ^ static_cast<value>(r));
	}
	friend constexpr Flags operator~(FlagType r) {
		return Flags(~static_cast<value>(r));
	}
};

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

	glm::vec4 transformHSV(glm::vec4 in,
	  float h,	// hue shift (in degrees)
	  float s, float v);

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
		void reset() { r.release(); }
	};

	template<typename T> class RepeatIterator
	{
	  public:
		using value_type = T;
		RepeatIterator() = default;
		RepeatIterator(const T* ptr) {
			static_assert(std::random_access_iterator<RepeatIterator>);
			_pos = 0;
			_ptr = ptr;
		};
		RepeatIterator(const T* ptr, size_t pos) {
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

	template<typename T> auto makeRepeatingView(const T& t, size_t n = 1) {
		return View<RepeatIterator<T>>(
		  RepeatIterator(&t), RepeatIterator(&t, n));
	}

	template<typename T> class OptionalUniquePtr
	{
		T* value = nullptr;

	  public:
		OptionalUniquePtr() = default;
		explicit OptionalUniquePtr(T* ptr) { value = ptr; }
		OptionalUniquePtr(const OptionalUniquePtr&) = delete;
		OptionalUniquePtr(OptionalUniquePtr&& other) {
			value = other.value;
			other.value = nullptr;
		}
		OptionalUniquePtr& operator=(OptionalUniquePtr&& other) {
			value = other.value;
			other.value = nullptr;
		}
		template<typename... CtorArgs> void make(CtorArgs&&... args) {
			delete value;
			value = new T(std::forward<CtorArgs>(args)...);
		}
		T* operator->() { return value; }
		const T* operator->() const { return value; }
		T& operator*() { return *value; }
		const T& operator*() const { return *value; }
		operator bool() const { return !isNull(); }
		bool isNull() const { return value == nullptr; }
		void reset() {
			delete value;
			value = nullptr;
		}
		~OptionalUniquePtr() { delete value; }
	};
	template<typename T, typename... CtorArgs>
	static OptionalUniquePtr<T> makeOptionalUniquePtr(CtorArgs&&... args) {
		return OptionalUniquePtr<T>(new T(std::forward<CtorArgs>(args)...));
	}

}





// Definition
#define _DEFA template<size_t AlignSize>
#define _DEFB StackMemoryResource<AlignSize>
#define _DEFCTOR _DEFA _DEFB

export namespace pmr {

	_DEFCTOR::StackMemoryResource():
	  StackMemoryResource(
		DEFAULT_BLOCK_SIZE, std::pmr::get_default_resource()) {}

	_DEFCTOR::StackMemoryResource(MemoryResource* upstream):
	  StackMemoryResource(DEFAULT_BLOCK_SIZE, upstream) {}

	_DEFCTOR::StackMemoryResource(size_t blockSize):
	  StackMemoryResource(blockSize, std::pmr::get_default_resource()) {}

	_DEFCTOR::StackMemoryResource(size_t blockSize, MemoryResource* upstream) {
		_upstream = upstream;
		_blockSize = blockSize;
		if ((_blockSize & (AlignSize - 1)) != 0)
			_blockSize = (_blockSize & ~(AlignSize - 1)) + AlignSize;
		if (_blockSize < 2 * sizeof(void*) + AlignSize) throw BadArgException{};
		assert(_blockSize % sizeof(size_t) == 0);
		assert(_blockSize >= DEFAULT_BLOCK_SIZE);
		_stackPtr = (Byte*)_upstream->allocate(
		  _blockSize + sizeof(Byte*) + alignUpTo(sizeof(Byte*), AlignSize),
		  AlignSize);
		*reinterpret_cast<Byte**>(_stackPtr) = nullptr;
		_stackPtr += alignUpTo(sizeof(Byte*), AlignSize);
		_currentBlockTop = _stackPtr + _blockSize;
	}
	_DEFCTOR::~StackMemoryResource() noexcept {
		release();
		Byte** base = reinterpret_cast<Byte**>(
		  _currentBlockTop - _blockSize - alignUpTo(sizeof(Byte*), AlignSize));
		_upstream->deallocate(base,
		  _blockSize + sizeof(Byte*) + alignUpTo(sizeof(Byte*), AlignSize));
	}

	_DEFA void _DEFB::release() {
		Byte** previousBlock = reinterpret_cast<Byte**>(
		  _currentBlockTop - _blockSize - alignUpTo(sizeof(Byte*), AlignSize));
		while (*previousBlock != nullptr) {
			_currentBlockTop = *previousBlock;
			_upstream->deallocate(previousBlock,
			  _blockSize + sizeof(Byte*) + alignUpTo(sizeof(Byte*), AlignSize),
			  AlignSize);
			previousBlock =
			  reinterpret_cast<Byte**>(_currentBlockTop - _blockSize
									   - alignUpTo(sizeof(Byte*), AlignSize));
		}
		_stackPtr = reinterpret_cast<Byte*>(previousBlock)
				  + alignUpTo(sizeof(Byte*), AlignSize);
	}

	_DEFA MemoryResource* _DEFB::upstreamResource() const { return _upstream; }
	_DEFA size_t _DEFB::blockSize() const { return _blockSize; }

	_DEFA void* _DEFB::do_allocate(size_t bytes, size_t alignment) {
		if (bytes == 0) UNLIKELY
		return _stackPtr;
		if (alignment <= AlignSize && bytes <= _blockSize) LIKELY {
				// Align requested data size
				if ((bytes & (AlignSize - 1)) != 0)
					bytes = (bytes & ~(AlignSize - 1)) + AlignSize;

				// alloc a place for data
				Byte* r = _stackPtr;
				if (_stackPtr + bytes > _currentBlockTop) UNLIKELY {
						// block does not have enough space. Call upstream
						// alloc. Should be able to safely throw bad alloc
						// without corrupting memory.
						Byte* newBlock = (Byte*)_upstream->allocate(
						  _blockSize + sizeof(Byte*)
							+ alignUpTo(sizeof(Byte*), AlignSize),
						  AlignSize);
						// setup the correct top of new block and place last
						// block's stack ptr to bottom of new block
						*reinterpret_cast<Byte**>(_currentBlockTop) = r;
						*reinterpret_cast<Byte**>(newBlock) = _currentBlockTop;
						// assign the data pos at after the bottom ptr;
						r = newBlock + alignUpTo(sizeof(Byte*), AlignSize);
						_currentBlockTop = r + _blockSize;
					}
				// assign the stack ptr to top of data
				_stackPtr = r + bytes;
				return r;
			}
		else [[unlikely]]
			throw BadAllocException{bytes, alignment};
	}
	_DEFA void _DEFB::do_deallocate(
	  void* p, size_t bytes, size_t alignment) noexcept {
		if (bytes == 0) UNLIKELY
		return;
		if (alignment <= AlignSize && bytes <= _blockSize) LIKELY {
				// Align requested data size
				if ((bytes & (AlignSize - 1)) != 0)
					bytes = (bytes & ~(AlignSize - 1)) + AlignSize;

				if (_stackPtr == _currentBlockTop - _blockSize) UNLIKELY {
						// Dealloc blocks and return to previous block
						Byte* base = _currentBlockTop - _blockSize
								   - alignUpTo(sizeof(Byte*), AlignSize);
						_currentBlockTop = *reinterpret_cast<Byte**>(base);
						_stackPtr = *reinterpret_cast<Byte**>(_currentBlockTop);
						_upstream->deallocate(base,
						  _blockSize + sizeof(Byte*)
							+ alignUpTo(sizeof(Byte*), AlignSize),
						  AlignSize);
					}

				if (_stackPtr == reinterpret_cast<Byte*>(p) + bytes) LIKELY {
						// Move back stack pointer
						_stackPtr = reinterpret_cast<Byte*>(p);
						return;
					}
			}
		DetectedCritcalMemoryCorruption();	// No Return
	}
	_DEFA bool _DEFB::do_is_equal(const MemoryResource& other) const noexcept {
		return &other == this;
	}
}

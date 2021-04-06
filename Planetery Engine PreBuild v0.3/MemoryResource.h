#pragma once
#include <memory_resource>
#include <assert.h>

#include "Define.h"

constexpr size_t DEFAULT_ALIGNMEMT = alignof(std::max_align_t);
constexpr size_t DEFAULT_BLOCK_SIZE = 256;

constexpr size_t alignUpTo(size_t p, size_t a) {
	return (p >= a && p % a == 0) ? p : (p + a - p % a);
}

namespace pmr {
	// exceptions
	struct BadAllocException {
		size_t requestedSize;
		size_t requestedAlignment;
		bool critical;
	};
	struct BadDeallocException {
		void* requestedPtr;
		size_t requestedSize;
		size_t requestedAlignment;
		bool critical;
	};





	using namespace std::pmr;
	using Byte = std::byte;
	template<typename T> using Allocator = polymorphic_allocator<T>;
	using MemoryResource = memory_resource;
	using MonotonicResource = monotonic_buffer_resource;

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
		  void* p, size_t bytes, size_t alignment) final;
		virtual bool do_is_equal(
		  const MemoryResource& other) const noexcept final;

		[[nodiscard]] void* allocate(
		  size_t bytes, size_t alignment = AlignSize) {
			return do_allocate(bytes, alignment);
		}

		void deallocate(void* p, size_t bytes, size_t alignment = AlignSize) {
			do_deallocate(p, bytes, alignment);
		}

		bool is_equal(const memory_resource& other) const noexcept {
			do_is_equal(other);
		}

		virtual ~StackMemoryResource() final;

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
		pmrAllocator(MR* memory_resource): mr(memory_resource) {}
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





// Definition
#define _DEFA	 template<size_t AlignSize>
#define _DEFB	 StackMemoryResource<AlignSize>
#define _DEFCTOR _DEFA _DEFB


	_DEFCTOR::StackMemoryResource():
	  StackMemoryResource(DEFAULT_BLOCK_SIZE, get_default_resource()) {}

	_DEFCTOR::StackMemoryResource(MemoryResource* upstream):
	  StackMemoryResource(DEFAULT_BLOCK_SIZE, upstream) {}

	_DEFCTOR::StackMemoryResource(size_t blockSize):
	  StackMemoryResource(blockSize, get_default_resource()) {}

	_DEFCTOR::StackMemoryResource(size_t blockSize, MemoryResource* upstream) {
		_upstream = upstream;
		_blockSize = blockSize;
		if ((_blockSize & (AlignSize - 1)) != 0)
			_blockSize = (_blockSize & ~(AlignSize - 1)) + AlignSize;
		if (_blockSize < 2 * sizeof(void*) + AlignSize)
			throw "pmr::InvalidArgs";
		assert(_blockSize % sizeof(size_t) == 0);
		assert(_blockSize >= DEFAULT_BLOCK_SIZE);
		_stackPtr = (Byte*)_upstream->allocate(
		  _blockSize + sizeof(Byte*) + alignUpTo(sizeof(Byte*), AlignSize),
		  AlignSize);
		*reinterpret_cast<Byte**>(_stackPtr) = nullptr;
		_stackPtr += alignUpTo(sizeof(Byte*), AlignSize);
		_currentBlockTop = _stackPtr + _blockSize;
	}
	_DEFCTOR::~StackMemoryResource() {
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
		if (bytes == 0) return _stackPtr;
		if (alignment <= AlignSize && bytes <= _blockSize) {
			// Align requested data size
			if ((bytes & (AlignSize - 1)) != 0)
				bytes = (bytes & ~(AlignSize - 1)) + AlignSize;
			assert(bytes % AlignSize == 0);
			// Save and move up stack pointer
			Byte* r = _stackPtr;
			_stackPtr += bytes;
			if (_stackPtr > _currentBlockTop) {
				// block out of space. Call upstream alloc for new block
				Byte* newBlock = (Byte*)_upstream->allocate(
				  _blockSize + sizeof(Byte*)
					+ alignUpTo(sizeof(Byte*), AlignSize),
				  AlignSize);
				*reinterpret_cast<Byte**>(_currentBlockTop) = r;
				*reinterpret_cast<Byte**>(newBlock) = _currentBlockTop;
				r = newBlock + alignUpTo(sizeof(Byte*), AlignSize);
				_currentBlockTop = r + _blockSize;
				_stackPtr = r + bytes;
			}
			return r;
		} else
			throw BadAllocException{bytes, alignment, false};
	}
	_DEFA void _DEFB::do_deallocate(void* p, size_t bytes, size_t alignment) {
		if (bytes == 0) return;
		if (alignment <= AlignSize && bytes <= _blockSize) {
			// Align requested data size
			if ((bytes & (AlignSize - 1)) != 0)
				bytes = (bytes & ~(AlignSize - 1)) + AlignSize;
			assert(bytes % AlignSize == 0);
			if (_stackPtr == _currentBlockTop - _blockSize) {
				// Dealloc blocks and return to previous block
				Byte* base = _currentBlockTop - _blockSize
						   - alignUpTo(sizeof(Byte*), AlignSize);
				_currentBlockTop = *reinterpret_cast<Byte**>(base);
				_stackPtr = *reinterpret_cast<Byte**>(_currentBlockTop);
				_upstream->deallocate(base,
				  _blockSize + sizeof(Byte*)
					+ alignUpTo(sizeof(Byte*), AlignSize),
				  AlignSize);
			} else if (_stackPtr < _currentBlockTop - _blockSize)
				throw BadDeallocException(p, bytes, alignment, true);
			else if (_stackPtr - bytes != p) {
				throw BadDeallocException(p, bytes, alignment, true);
			}
			// Move back stack pointer
			_stackPtr -= bytes;
		} else
			throw BadDeallocException(p, bytes, alignment, false);
	}
	_DEFA bool _DEFB::do_is_equal(const MemoryResource& other) const noexcept {
		return &other == this;
	}
}

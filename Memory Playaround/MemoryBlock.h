#pragma once

// (for <=8 bytes)
// 2, 4, 8, 16
// 32, 64, 128, 256
// 512, 1024, 2048, 4096
// 8192, 16384, 32768, 65536 (MAX)
// (for <=16 bytes)
// -- 65536
// (for <=32 bytes)
//

//set value:

#include "Using.h"

#include <cmath>
#include <array>



constexpr ulint c_fastlog2(ulint x) {
	int n = 64;
	ulint y;

	y = x >> 32; if (y != 0) { n = n - 32; x = y; }
	y = x >> 16; if (y != 0) { n = n - 16; x = y; }
	y = x >> 8; if (y != 0) { n = n - 8; x = y; }
	y = x >> 4; if (y != 0) { n = n - 4; x = y; }
	y = x >> 2; if (y != 0) { n = n - 2; x = y; }
	y = x >> 1; if (y != 0) return 65 - n;
	return 63 + x - n;
}
constexpr size_t c_fastexp2(size_t n) {
	return ulint(1) << n;
}
constexpr size_t c_dataScale(size_t n) {
	return 19 - c_fastlog2(n);
}
constexpr size_t c_maxBlockSize(size_t n) {
	return c_fastexp2(c_dataScale(n));
}
constexpr size_t c_locatedBlockA(size_t n) {
	return c_fastlog2(n + 2) - 1; //not yet thought about maxBlockSize
}
constexpr size_t c_offsetBlockA(size_t n) {
	return (n + 2 - c_fastexp2(c_locatedBlockA(n) + 1));
}

inline ulint fastlog2(ulint x) {
	return 63 - __lzcnt64(x);
}
inline size_t fastexp2(size_t n) {
	return ulint(1) << n;
}
inline size_t dataScale(size_t n) {
	return 19 - fastlog2(n);
}
inline size_t maxBlockSize(size_t n) {
	return fastexp2(dataScale(n));
}
inline size_t locatedBlockA(size_t n) {
	return fastlog2(n + 2) - 1; //not yet thought about maxBlockSize
}
inline size_t slocatedBlockA(size_t n) {
	return 62 - __lzcnt64(n + 2); //not yet thought about maxBlockSize
}
inline size_t offsetBlockA(size_t n) {
	return (n + 2 - fastexp2(locatedBlockA(n) + 1));
}
inline ulint soffsetBlockA(ulint n) {
	return (n + 2 - (ulint(1) << (63 - __lzcnt64(n + 2))));
}
inline ulint foffsetBlockA(ulint n, ulint l) {
	return (n + 2 - (ulint(1) << (l + 1)));
}
inline ulint fstartBlockA(ulint i) {
	return (ulint(1) << (i+1))-2;
}

template <size_t N>
class MemoryBlockA {
public:
	std::array<char*, c_dataScale(N)> blocks;
	std::array<ulint, c_dataScale(N)> cache;
	size_t top;

	MemoryBlockA() {
		blocks.fill(nullptr);
		top = 0;
		blocks[0] = new char[N*2];
		for (uint i = 0; i < cache.size(); i++) {
			cache[i] = fstartBlockA(i);
		}
	}

	~MemoryBlockA() {
		for (ulint i = 0; i < blocks.size(); i++) {
			if (blocks[i] != nullptr) {
				delete blocks[i], N* fastexp2(i + 1);
			}
		}
	}

	void* operator[](size_t i) {
		size_t blockI = slocatedBlockA(i);
		return (blocks[blockI]) + ((i - cache[blockI]) * N);
	}
	void* extend() {
		size_t blockI = slocatedBlockA(top);
		size_t offset = top - cache[blockI];
		if (blocks[blockI] == nullptr) {
			blocks[blockI] = new char[N * fastexp2(blockI + 1)];
		}
		top++;
		return (blocks[blockI]) + (offset * N);
	}
	void retract() {
		top--;
	}
};

template <size_t N>
class MemoryBlockB {
public:
	std::vector<char*> blocks;
	std::vector<ulint> startOffset;
	size_t size;
	size_t top;
	size_t topSize;

	MemoryBlockB() {
		top = 0;
		startOffset.push_back(0);
		topSize = 2;
		blocks.push_back(new char[N * topSize]);
		startOffset.push_back(topSize);
		size = 1;
	}

	~MemoryBlockB() {
		for (uint i = size; i > 0; i--) {
			delete blocks[i - 1], N* topSize;
			topSize /= 2;
		}
	}

	inline uint findBlock(ulint n) {
		uint i = size;
		while (startOffset[i] > n) {
			i--;
		}
		return i;
	}

	void* operator[](size_t i) {
		size_t blockI = findBlock(i);
		return (blocks[blockI]) + ((i - startOffset[blockI]) * N);
	}
	void* extend() {
		size_t blockI = findBlock(top);
		if (blockI >= size) {
			topSize *= 2;
			blocks.push_back(new char[N * topSize]);
			startOffset.push_back(startOffset[size - 1] + topSize);
			size++;
		}
		size_t offset = top - startOffset[blockI];
		top++;
		return (blocks[blockI]) + (offset * N);
	}
	void retract() {
		top--;
	}
};

template <size_t N>
class MemoryBlockC {
public:
	std::vector<char*> blocks;
	std::vector<ulint> blockSize;
	size_t size;
	size_t index;
	lint currentBlock;
	lint currentOffset;

	MemoryBlockC() {
		currentBlock = 0;
		currentOffset = 0;
		index = 0;
		blockSize.push_back(64);
		blocks.push_back(new char[N * 64]);
		size = 1;
	}

	~MemoryBlockC() {
		for (uint i = size; i > 0; i--) {
			delete blocks[i - 1], blockSize[i - 1];
		}
	}

	void* operator[](size_t i) {
		lint offset = i - index;
		if (offset > blockSize[currentBlock]) {
			currentBlock = 0;

			ulint total = blockSize[currentBlock];
			while (total <= i) {
				currentBlock++;
				total += blockSize[currentBlock];
			}
			currentOffset = blockSize[currentBlock] - (total - i);
		}
		else {
			currentOffset += offset;
			if (currentOffset < 0) {
				currentOffset += blockSize[currentBlock-1];
				currentBlock--;
			}
			else if (currentOffset >= blockSize[currentBlock]) {
				currentOffset -= blockSize[currentBlock];
				currentBlock++;
			}
		}
		index = i;
		return blocks[currentBlock] + (currentOffset * N);
	}

	void* extend() {
		char* result = blocks[currentBlock] + (currentOffset * N);
		index++;
		currentOffset++;
		if (currentOffset >= blockSize[currentBlock]) {
			blockSize.push_back(currentOffset * 2);
			currentBlock++;
			if (currentBlock >= size) {
				blocks.push_back(new char[N * currentOffset * 2]);
				size++;
			}
			currentOffset = 0;
		}
		return result;
	}

	void retract() {
		
	}
};

inline size_t slocatedBlockB(size_t n) {
	return 64 - __lzcnt64(n); //not yet thought about maxBlockSize
}
inline ulint fstartBlockB(ulint i) {
	return (ulint(1) << i) >> 1;
}
inline size_t fastexp2B(size_t n) {
	return n ? (ulint(1) << (n-1)) : 1;
}
template <size_t N>
class MemoryBlockD {
public:
	std::vector<char*> blocks;
	std::vector<ulint> cache;
	size_t top;

	MemoryBlockD() {
		top = 0;
		blocks.push_back(new char[N]);
		cache.push_back(fstartBlockB(0));
	}

	~MemoryBlockD() {
		for (ulint i = 0; i < blocks.size(); i++) {
			if (blocks[i] != 0) {
				delete blocks[i], N* fastexp2B(i);
			}
		}
	}

	inline void* operator[](size_t i) {
		size_t blockI = slocatedBlockB(i);
		return (blocks[blockI]) + ((i - cache[blockI]) * N);
	}
	inline void* extend() {
		size_t blockI = slocatedBlockB(top);
		if (blockI >= blocks.size()) {
			blocks.push_back(new char[N * fastexp2B(blockI)]);
			cache.push_back(fstartBlockB(blockI));
		}
		size_t offset = top - cache[blockI];
		top++;
		return (blocks[blockI]) + (offset * N);
	}
	inline void retract() {
		top--;
	}
};
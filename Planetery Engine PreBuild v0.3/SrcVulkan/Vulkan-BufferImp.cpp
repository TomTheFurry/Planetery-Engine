module;
#include "Marco.h"
module Vulkan: BufferImp;
import: Buffer;
import: Device;
import: Enum;
import: Commend;
import: Memory;
import: Lifetime;
import: Sync;
import std.core;
import Define;
import Logger;
import "Assert.h";
import "VulkanExtModule.h";
using namespace vk;

void Buffer::_setup() {
	VkBufferCreateInfo bInfo{};
	bInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bInfo.size = size;
	bInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bInfo.usage = (VkBufferUsageFlags)usage;
	if (vkCreateBuffer(d.d, &bInfo, nullptr, &b) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(d.d, b, &memRequirements);
	mSize = memRequirements.size;
	auto pair = d.allocMemory(
	  memRequirements.memoryTypeBits, feature, mSize, minAlignment);
	memoryIndex = pair.first;
	ptr = pair.second;

	if (vkBindBufferMemory(d.d, b, ptr.dm.dm, ptr.offset) != VK_SUCCESS)
		throw "VulkanBufferBindMemoryFailure";
}

Buffer::Buffer(LogicalDevice& device, size_t s,
  Flags<MemoryFeature> neededFeature, Flags<BufferUseType> neededUsage):
  d(device) {
	size = s;
	feature = neededFeature;
	usage = neededUsage;
	if (feature.has(MemoryFeature::Mappable)) {
		if (feature.has(MemoryFeature::Coherent))
			minAlignment =
			  d.pd.properties10.properties.limits.nonCoherentAtomSize;
		else
			minAlignment =
			  d.pd.properties10.properties.limits.minMemoryMapAlignment;
	} else {
		minAlignment = 1;
	}
	if (feature.has(MemoryFeature::IndirectReadable)) {
		usage.set(BufferUseType::TransferSrc);
	}
	if (feature.has(MemoryFeature::IndirectWritable)) {
		usage.set(BufferUseType::TransferDst);
	}
	// HOTFIX: This is hotfix for min requirement for alignment
	minAlignment = d.pd.properties10.properties.limits.nonCoherentAtomSize;
	_setup();
}

Buffer::Buffer(Buffer&& other) noexcept: d(other.d) {
	mSize = other.size;
	size = other.size;
	minAlignment = other.minAlignment;
	b = other.b;
	memoryIndex = other.memoryIndex;
	ptr = other.ptr;
	mappedPtr = other.mappedPtr;
	feature = other.feature;
	usage = other.usage;
	other.b = nullptr;
	other.memoryIndex = uint(-1);
	other.mappedPtr = nullptr;
}

Buffer::~Buffer() {
	if (b != nullptr) vkDestroyBuffer(d.d, b, nullptr);
	if (memoryIndex != uint(-1)) d.freeMemory(memoryIndex, ptr);
}
void* Buffer::map() {
	if constexpr (DO_SAFETY_CHECK) {
		if (!feature.has(MemoryFeature::Mappable))
			throw "VulkanBufferNotMappable";
		if (mappedPtr != nullptr) throw "VulkanBufferAlreadyMapped";
	}
	vkMapMemory(d.d, ptr.dm.dm, ptr.offset, mSize, 0, &mappedPtr);
	return mappedPtr;
}
void* Buffer::map(size_t nSize, size_t offset) {
	if constexpr (DO_SAFETY_CHECK) {
		if (nSize + offset > size) throw "VulkanBufferOutOfRange";
		if (!feature.has(MemoryFeature::Mappable))
			throw "VulkanBufferNotMappable";
		if (mappedPtr != nullptr) throw "VulkanBufferAlreadyMapped";
		if (nSize % minAlignment != 0) throw "VulkanBufferNotOnMinAlignment";
		if (offset % minAlignment != 0) throw "VulkanBufferNotOnMinAlignment";
	}
	vkMapMemory(d.d, ptr.dm.dm, ptr.offset + offset, nSize, 0, &mappedPtr);
	return mappedPtr;
}
void Buffer::flush() {
	if constexpr (DO_SAFETY_CHECK) {
		if (!feature.has(MemoryFeature::Mappable))
			throw "VulkanBufferNotMappable";
		if (mappedPtr == nullptr) throw "VulkanBufferNotMapped";
		if (feature.has(MemoryFeature::Coherent))
			logger("Warning: Vulkan called flush() on coherent buffer.\n");
	}
	VkMappedMemoryRange r{};
	r.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	r.memory = ptr.dm.dm;
	r.offset = ptr.offset;
	r.size = mSize;
	vkFlushMappedMemoryRanges(d.d, 1, &r);
}
void Buffer::flush(size_t nSize, size_t offset) {
	if constexpr (DO_SAFETY_CHECK) {
		if (nSize + offset > size) throw "VulkanBufferOutOfRange";
		if (!feature.has(MemoryFeature::Mappable))
			throw "VulkanBufferNotMappable";
		if (mappedPtr == nullptr) throw "VulkanBufferNotMapped";
		if (nSize % minAlignment != 0) throw "VulkanBufferNotOnMinAlignment";
		if (offset % minAlignment != 0) throw "VulkanBufferNotOnMinAlignment";
	}
	VkMappedMemoryRange r{};
	r.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	r.memory = ptr.dm.dm;
	r.size = nSize;
	r.offset = ptr.offset + offset;
	vkFlushMappedMemoryRanges(d.d, 1, &r);
}
void Buffer::unmap() {
	if constexpr (DO_SAFETY_CHECK) {
		if (!feature.has(MemoryFeature::Mappable))
			throw "VulkanBufferNotMappable";
		if (mappedPtr == nullptr) throw "VulkanBufferNotMapped";
		mappedPtr = nullptr;
	}
	// FIXME: Can't unmap part of memory...
	vkUnmapMemory(d.d, ptr.dm.dm);
}

void Buffer::cmdIndirectWrite(
  LifetimeManager& commendLifetime, CommendBuffer& cb, void* data) {
	if constexpr (DO_SAFETY_CHECK) {
		if (!feature.has(MemoryFeature::IndirectWritable))
			throw "VulkanBufferNotIndirectWritable";
	}
	auto& stagingBuffer = commendLifetime.make<Buffer>(d, size,
	  Flags(MemoryFeature::Mappable) | MemoryFeature::IndirectReadable);
	memcpy(stagingBuffer.map(), data, size);
	stagingBuffer.unmap();
	cb.cmdCopy(stagingBuffer, *this, size);
}
void Buffer::cmdIndirectWrite(LifetimeManager& commendLifetime,
  CommendBuffer& cb, size_t nSize, size_t offset, void* data) {
	if constexpr (DO_SAFETY_CHECK) {
		if (!feature.has(MemoryFeature::IndirectWritable))
			throw "VulkanBufferNotIndirectWritable";
		if (nSize + offset > size) throw "VulkanBufferOutOfRange";
	}
	auto& stagingBuffer = commendLifetime.make<Buffer>(d, size,
	  Flags(MemoryFeature::Mappable) | MemoryFeature::IndirectReadable);
	memcpy(stagingBuffer.map(), data, nSize);
	stagingBuffer.unmap();
	cb.cmdCopy(stagingBuffer, *this, nSize, 0, offset);
}
void Buffer::blockingIndirectWrite(const void* data) {
	if constexpr (DO_SAFETY_CHECK) {
		if (!feature.has(MemoryFeature::IndirectWritable))
			throw "VulkanBufferNotIndirectWritable";
		// TODO: Add queue & stagingCmdPool Check
	}
	auto sg = Buffer(d, size,
	  Flags(MemoryFeature::Mappable) | MemoryFeature::IndirectReadable);
	sg.directWrite(data);
	auto& q = d.getQueuePool().getExpressMemoryCopyQueue();
	auto& cp = d.getQueuePool().queryCommendPool(
	  q.familyIndex, CommendPoolType::Shortlived);
	auto cb = cp.makeCommendBuffer();
	cb.startRecording(CommendBufferUsage::Streaming);
	cb.cmdCopy(sg, *this, size);
	cb.endRecording();
	// TODO: Make this able to return a 'future' like object to wait on
	cb.quickSubmit(q).wait();
}
void Buffer::blockingIndirectWrite(
  size_t nSize, size_t offset, const void* data) {
	if constexpr (DO_SAFETY_CHECK) {
		if (!feature.has(MemoryFeature::IndirectWritable))
			throw "VulkanBufferNotIndirectWritable";
		if (nSize + offset > size) throw "VulkanBufferOutOfRange";
		// TODO: Add queue & stagingCmdPool Check
	}
	auto sg = Buffer(d, nSize,
	  Flags(MemoryFeature::Mappable) | MemoryFeature::IndirectReadable);
	sg.directWrite(data);
	auto& q = d.getQueuePool().getExpressMemoryCopyQueue();
	auto& cp = d.getQueuePool().queryCommendPool(
	  q.familyIndex, CommendPoolType::Shortlived);
	auto cb = cp.makeCommendBuffer();
	cb.startRecording(CommendBufferUsage::Streaming);
	cb.cmdCopy(sg, *this, nSize, 0, offset);
	cb.endRecording();
	// TODO: Make this able to return a 'future' like object to wait on
	cb.quickSubmit(q).wait();
}
void Buffer::directWrite(const void* data) {
	memcpy(map(), data, size);
	flush();
	unmap();
}

void Buffer::directWrite(size_t nSize, size_t offset, const void* data) {
	memcpy(map(nSize, offset), data, nSize);
	flush();
	unmap();
}

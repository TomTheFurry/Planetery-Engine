module;
#include "Marco.h"
module Vulkan: BufferImp;
import: Buffer;
import: Device;
import: Enum;
import: Tick;
import: Commend;
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
	uint memTypeIndex =
	  d.pd.getMemoryTypeIndex(memRequirements.memoryTypeBits, feature);
	// TODO: Add fallback
	if (memTypeIndex == -1) throw "VulkanFailedToGetMemoryType";
	// TODO: Fix allocator for custom allocation in PhysicalDevice
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = memTypeIndex;
	if (vkAllocateMemory(d.d, &allocInfo, nullptr, &dm) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}
	vkBindBufferMemory(d.d, b, dm, 0);
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
	_setup();
}

Buffer::Buffer(Buffer&& other) noexcept: d(other.d) {
	size = other.size;
	minAlignment = other.minAlignment;
	b = other.b;
	dm = other.dm;
	mappedPtr = other.mappedPtr;
	feature = other.feature;
	usage = other.usage;
	other.b = nullptr;
	other.dm = nullptr;
	other.mappedPtr = nullptr;
}

Buffer::~Buffer() {
	if (b != nullptr) vkDestroyBuffer(d.d, b, nullptr);
	if (dm != nullptr) vkFreeMemory(d.d, dm, nullptr);
}
void* Buffer::map() {
	if constexpr (DO_SAFETY_CHECK) {
		if (!feature.has(MemoryFeature::Mappable))
			throw "VulkanBufferNotMappable";
		if (mappedPtr != nullptr) throw "VulkanBufferAlreadyMapped";
	}
	vkMapMemory(d.d, dm, 0, VK_WHOLE_SIZE, 0, &mappedPtr);
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
	vkMapMemory(d.d, dm, offset, nSize, 0, &mappedPtr);
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
	r.memory = dm;
	r.offset = 0;
	r.size = VK_WHOLE_SIZE;
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
	r.memory = dm;
	r.size = nSize;
	r.offset = offset;
	vkFlushMappedMemoryRanges(d.d, 1, &r);
}
void Buffer::unmap() {
	if constexpr (DO_SAFETY_CHECK) {
		if (!feature.has(MemoryFeature::Mappable))
			throw "VulkanBufferNotMappable";
		if (mappedPtr == nullptr) throw "VulkanBufferNotMapped";
		mappedPtr = nullptr;
	}
	vkUnmapMemory(d.d, dm);
}

void Buffer::cmdIndirectWrite(RenderTick& rt, CommendBuffer& cb, void* data) {
	if constexpr (DO_SAFETY_CHECK) {
		if (!feature.has(MemoryFeature::IndirectWritable))
			throw "VulkanBufferNotIndirectWritable";
	}
	auto& s = rt.makeStagingBuffer(size);
	memcpy(s.map(), data, size);
	s.unmap();
	cb.cmdCopy(s, *this, size);
}
void Buffer::cmdIndirectWrite(
  RenderTick& rt, CommendBuffer& cb, size_t nSize, size_t offset, void* data) {
	if constexpr (DO_SAFETY_CHECK) {
		if (!feature.has(MemoryFeature::IndirectWritable))
			throw "VulkanBufferNotIndirectWritable";
		if (nSize + offset > size) throw "VulkanBufferOutOfRange";
	}
	auto& s = rt.makeStagingBuffer(nSize);
	memcpy(s.map(), data, nSize);
	s.unmap();
	cb.cmdCopy(s, *this, nSize, 0, offset);
}
Buffer& Buffer::getStagingBuffer(RenderTick& rt) {
	if constexpr (DO_SAFETY_CHECK) {
		if (!feature.has(MemoryFeature::IndirectWritable))
			throw "VulkanBufferNotIndirectWritable";
	}
	return rt.makeStagingBuffer(size);
}
Buffer& Buffer::getStagingBuffer(RenderTick& rt, size_t nSize) {
	if constexpr (DO_SAFETY_CHECK) {
		if (!feature.has(MemoryFeature::IndirectWritable))
			throw "VulkanBufferNotIndirectWritable";
		if (nSize > size) throw "VulkanBufferOutOfRange";
	}
	return rt.makeStagingBuffer(size);
}
void Buffer::blockingIndirectWrite(const void* data) {
	if constexpr (DO_SAFETY_CHECK) {
		if (!feature.has(MemoryFeature::IndirectWritable))
			throw "VulkanBufferNotIndirectWritable";
	}
	auto sg = Buffer(d, size,
	  Flags(MemoryFeature::Mappable) | MemoryFeature::IndirectReadable);
	sg.directWrite(data);
	auto cb = d.getSingleUseCommendBuffer();
	cb.startRecording(CommendBufferUsage::Streaming);
	cb.cmdCopy(sg, *this, size);
	cb.endRecording();
	cb.submit().wait();
}
void Buffer::blockingIndirectWrite(size_t nSize, size_t offset, const void* data) {
	if constexpr (DO_SAFETY_CHECK) {
		if (!feature.has(MemoryFeature::IndirectWritable))
			throw "VulkanBufferNotIndirectWritable";
		if (nSize + offset > size) throw "VulkanBufferOutOfRange";
	}
	auto sg = Buffer(d, nSize,
	  Flags(MemoryFeature::Mappable) | MemoryFeature::IndirectReadable);
	sg.directWrite(data);
	auto cb = d.getSingleUseCommendBuffer();
	cb.startRecording(CommendBufferUsage::Streaming);
	cb.cmdCopy(sg, *this, nSize, 0, offset);
	cb.endRecording();
	cb.submit().wait();
}
void Buffer::directWrite(const void* data) 	{
	memcpy(map(), data, size);
	flush(); //FIXME: ????????? Is this needed?
	unmap();
}

void Buffer::directWrite(size_t nSize, size_t offset, const void* data) {
	memcpy(map(nSize, offset), data, nSize);
	flush(); //FIXME: ????????? Is this needed?
	unmap();
}


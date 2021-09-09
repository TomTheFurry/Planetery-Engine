export module Vulkan: Buffer;
export import: Declaration;
import: Enum;
import: Memory;
import Util;
import "VulkanExtModule.h";

// Buffer class:
export namespace vk {
	// FIXME: Can't unmap part of memory...
	// Effect: Only allowed to map one memory at a time
	class Buffer : public ComplexObject
	{
		void _setup();

	  public:
		Buffer(vk::LogicalDevice& d, size_t size,
		  Flags<MemoryFeature> neededFeature = MemoryFeature::None,
		  Flags<BufferUseType> neededUsage = BufferUseType::None);
		Buffer(const Buffer&) = delete;
		Buffer(Buffer&& other) noexcept;
		~Buffer();
		void* map();
		void* map(size_t size, size_t offset);
		void flush();
		void flush(size_t size, size_t offset);
		void unmap();
		void cmdIndirectWrite(RenderTick& rt, CommendBuffer& cp, void* data);
		void cmdIndirectWrite(RenderTick& rt, CommendBuffer& cp, size_t size,
		  size_t offset, void* data);
		Buffer& getStagingBuffer(RenderTick& rt);
		Buffer& getStagingBuffer(RenderTick& rt, size_t size);
		void blockingIndirectWrite(const void* data);
		void blockingIndirectWrite(
		  size_t size, size_t offset, const void* data);
		void directWrite(const void* data);
		void directWrite(size_t size, size_t offset, const void* data);

		// TODO: Make a remake() function
		size_t mSize;
		size_t size;
		size_t minAlignment;
		Flags<MemoryFeature> feature;
		Flags<BufferUseType> usage;
		VkBuffer b = nullptr;
		uint memoryIndex;
		MemoryPointer ptr;
		void* mappedPtr = nullptr;
		LogicalDevice& d;
	};
	class VertexBuffer: public Buffer
	{
	  public:
		VertexBuffer(LogicalDevice& d, size_t size,
		  Flags<MemoryFeature> neededFeature = MemoryFeature::None,
		  Flags<BufferUseType> neededUsage = BufferUseType::None):
		  Buffer(d, size, neededFeature, neededUsage | BufferUseType::Vertex) {}
	};
	class IndexBuffer: public Buffer
	{
	  public:
		IndexBuffer(LogicalDevice& d, size_t size,
		  Flags<MemoryFeature> neededFeature = MemoryFeature::None,
		  Flags<BufferUseType> neededUsage = BufferUseType::None):
		  Buffer(d, size, neededFeature, neededUsage | BufferUseType::Index) {}
	};
	class UniformBuffer: public Buffer
	{
	  public:
		UniformBuffer(LogicalDevice& d, size_t size,
		  Flags<MemoryFeature> neededFeature = MemoryFeature::None,
		  Flags<BufferUseType> neededUsage = BufferUseType::None):
		  Buffer(d, size, neededFeature, neededUsage | BufferUseType::Uniform) {
		}
	};
	class StorageBuffer: public Buffer
	{
	  public:
		StorageBuffer(LogicalDevice& d, size_t size,
		  Flags<MemoryFeature> neededFeature = MemoryFeature::None,
		  Flags<BufferUseType> neededUsage = BufferUseType::None):
		  Buffer(d, size, neededFeature, neededUsage | BufferUseType::Storage) {
		}
	};
}

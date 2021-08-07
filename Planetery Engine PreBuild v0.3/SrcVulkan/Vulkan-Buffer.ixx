export module Vulkan: Buffer;
export import: Internal;
import: Enum;
import Util;
import "VulkanExtModule.h";

// Buffer class:
export namespace vk {
	class Buffer
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
		size_t size;
		size_t minAlignment;
		Flags<MemoryFeature> feature;
		Flags<BufferUseType> usage;
		VkBuffer b = nullptr;
		VkDeviceMemory dm = nullptr;
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
}

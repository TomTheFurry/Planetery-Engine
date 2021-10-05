export module Vulkan: Buffer;
export import: Declaration;
import: Enum;
import: Memory;
import Util;
import "VulkanExtModule.h";

export namespace vk {
	// FIXME: Can't unmap part of memory...
	// Effect: Only allowed to map one memory at a time

	/// <summary>
	/// Vulkan Buffer Base Class
	/// Provides basic universal functions for all types of buffers
	/// </summary>
	class Buffer: public ComplexObject
	{
		void _setup();

	  public:
		/// <summary>
		/// Buffer Constructer
		/// </summary>
		/// <param name="d">The Logical Device</param>
		/// <param name="size">Size of the Buffer</param>
		/// <param name="neededFeature">What memory feature is needed for this
		/// Buffer. See MemoryFeature </param> <param name="neededUsage">How
		/// this Buffer is going to be used</param>
		Buffer(vk::LogicalDevice& d, size_t size,
		  Flags<MemoryFeature> neededFeature = MemoryFeature::None,
		  Flags<BufferUseType> neededUsage = BufferUseType::None);
		/// <summary>
		/// Copy is disabled
		/// </summary>
		Buffer(const Buffer&) = delete;
		/// <summary>
		/// Move is allowed only if it is a unique reference
		/// </summary>
		Buffer(Buffer&& other) noexcept;
		/// <summary>
		/// Buffer Deleter
		/// </summary>
		/// Buffer should not be in use when deleter is called
		~Buffer();
		/// <summary>
		/// Map the Buffer for direct access
		/// </summary>
		/// Requires that on construction, the MemoryFeature::Mappable is set.
		/// External syncing of accessed memory is required.
		/// <returns>
		/// Raw pointer to the start of the buffer
		/// </returns>
		/// @bug
		/// Currently only one mapped region is allowed at a time due to
		/// shared memory region issues.
		void* map();
		void* map(size_t size, size_t offset);
		void flush();
		void flush(size_t size, size_t offset);
		void unmap();

		void cmdIndirectWrite(
		  LifetimeManager& commendLifetime, CommendBuffer& cb, void* data);
		void cmdIndirectWrite(LifetimeManager& commendLifetime,
		  CommendBuffer& cb, size_t size, size_t offset, void* data);

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

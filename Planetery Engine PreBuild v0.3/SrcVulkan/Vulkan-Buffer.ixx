export module Vulkan: Buffer;
export import: Declaration;
import: Enum;
import: Memory;
import Util;
import "VulkanExtModule.h";

export namespace vk {

	/// @addtogroup vkBuffer Buffer
	/// @ingroup vulkan
	/// @brief A group of vulkan buffer based classes
	///
	/// Provides multiple different buffer using class inheritance
	/// 
	/// @todo Better Buffer group docs...
	///
	/// @bug @anchor vkBufferBugMapping Cannot unmap just a buffer if said
	/// buffer shares same memory with another buffer. Current bypass is to only
	/// allow mapping to one buffer at a time
	/// @{

	/// @brief Vulkan Buffer Base Class
	///
	/// Provides basic universal functions for all types of buffers. Can be an
	/// object of its own if user wish to manage it on its own
	class Buffer: public ComplexObject
	{
		void _setup();

	  public:
		/// @brief Buffer Constructer
		///
		/// Construct a Buffer using the input parameters.\n\n If \c
		/// 'neededFeature' has MemoryFeature::IndirectWritable,
		/// BufferUseType::TransferDst is set.\n
		/// if \c 'neededFeature' has MemoryFeature::IndirectReadable,
		/// BufferUseType::TransferSrc is set.
		///
		/// @param d The Logical Device
		/// @param size Size of the Buffer
		/// @param neededFeature What @link MemoryFeature memory feature
		/// @endlink is needed for this Buffer
		/// @param neededUsage How this Buffer is going to be @link
		/// BufferUseType used @endlink
		Buffer(vk::LogicalDevice& d, size_t size,
		  Flags<MemoryFeature> neededFeature = MemoryFeature::None,
		  Flags<BufferUseType> neededUsage = BufferUseType::None);
		Buffer(const Buffer&) = delete;
		/// Move is allowed only if it is a unique reference
		Buffer(Buffer&& other) noexcept;
		/// @brief Buffer Deleter
		///
		/// Buffer should not be in use when deleter is called
		~Buffer();
		/// Map the Buffer for direct access
		/// @note Requires that on construction, the MemoryFeature::Mappable is
		/// set.
		/// @note External syncing of accessed memory is required.
		/// @return Raw pointer to the start of the buffer
		///
		/// @bug See @ref vkBufferBugMapping "memory mapping bug".
		void* map();
		/// @copydoc map()
		void* map(size_t size, size_t offset);
		/// Flush and sync the mapped Buffer memory
		/// @note If MemoryFeature::Coherent is set, call to flush() is not
		/// required
		void flush();
		/// @copydoc flush()
		void flush(size_t size, size_t offset);
		/// Unmap a mapped Buffer
		void unmap();

		/// Commend for indirectly writing to the Buffer data
		void cmdIndirectWrite(
		  LifetimeManager& commendLifetime, CommendBuffer& cb, void* data);
		/// copydoc cmdIndirectWrite(LifetimeManager&,CommendBuffer&,void*)
		void cmdIndirectWrite(LifetimeManager& commendLifetime,
		  CommendBuffer& cb, size_t size, size_t offset, void* data);

		/// Indirectly writes to the Buffer data (blocking)
		void blockingIndirectWrite(const void* data);
		/// copydoc blockingIndirectWrite(const void*)
		void blockingIndirectWrite(
		  size_t size, size_t offset, const void* data);

		/// Directly writes to the buffer data
		void directWrite(const void* data);
		/// copydoc directWrite(const void*)
		void directWrite(size_t size, size_t offset, const void* data);

		// TODO: Make a remake() function
		/// @todo hide this
		size_t mSize;
		/// @todo hide this
		size_t size;
		/// @todo hide this
		size_t minAlignment;
		/// @todo hide this
		Flags<MemoryFeature> feature;
		/// @todo hide this
		Flags<BufferUseType> usage;
		/// @todo hide this
		VkBuffer b = nullptr;
		/// @todo hide this
		uint memoryIndex;
		/// @todo hide this
		MemoryPointer ptr;
		/// @todo hide this
		void* mappedPtr = nullptr;
		/// @todo hide this
		LogicalDevice& d;
	};

	/// A vertex buffer
	class VertexBuffer: public Buffer
	{
	  public:
		/// See inherited function Buffer::Buffer()
		VertexBuffer(LogicalDevice& d, size_t size,
		  Flags<MemoryFeature> neededFeature = MemoryFeature::None,
		  Flags<BufferUseType> neededUsage = BufferUseType::None):
		  Buffer(d, size, neededFeature, neededUsage | BufferUseType::Vertex) {}
	};

	/// An index buffer
	class IndexBuffer: public Buffer
	{
	  public:
		/// See inherited function Buffer::Buffer()
		IndexBuffer(LogicalDevice& d, size_t size,
		  Flags<MemoryFeature> neededFeature = MemoryFeature::None,
		  Flags<BufferUseType> neededUsage = BufferUseType::None):
		  Buffer(d, size, neededFeature, neededUsage | BufferUseType::Index) {}
	};

	/// An uniform buffer
	class UniformBuffer: public Buffer
	{
	  public:
		/// See inherited function Buffer::Buffer()
		UniformBuffer(LogicalDevice& d, size_t size,
		  Flags<MemoryFeature> neededFeature = MemoryFeature::None,
		  Flags<BufferUseType> neededUsage = BufferUseType::None):
		  Buffer(d, size, neededFeature, neededUsage | BufferUseType::Uniform) {
		}
	};

	/// A shader storage buffer
	class StorageBuffer: public Buffer
	{
	  public:
		/// See inherited function Buffer::Buffer()
		StorageBuffer(LogicalDevice& d, size_t size,
		  Flags<MemoryFeature> neededFeature = MemoryFeature::None,
		  Flags<BufferUseType> neededUsage = BufferUseType::None):
		  Buffer(d, size, neededFeature, neededUsage | BufferUseType::Storage) {
		}
	};

	///@}
}

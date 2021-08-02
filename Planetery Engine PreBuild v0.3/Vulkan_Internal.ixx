module;
#include "Marco.h"
#pragma warning(disable : 26812)
#include <vulkan/vulkan.h>
export module Vulkan: Internal;
import: Enum;
import std.core;
import Define;
import Util;

// Class Declaration:
export namespace vk {
	// Device class:
	struct Layer;
	struct Extension;
	typedef uint QueueFamilyIndex;
	class PhysicalDevice;
	class LogicalDevice;
	class OSRenderSurface;
	class SwapChainSupport;
	class SwapChain;
	// Buffer class:
	class Buffer;
	class VertexBuffer;
	class IndexBuffer;
	class UniformBuffer;
	// Image class:
	class FrameBuffer;
	class ImageView;
	class Image;
	// Commend class:
	class CommendPool;
	class CommendBuffer;
	// Sync class:
	struct SyncLine;
	struct SyncPoint;
	typedef ulint SyncNumber;
	class Fence;
	class Semaphore;
	class TimelineSemaphore;
	// Descriptor class:
	struct DescriptorLayoutBinding;
	class DescriptorLayout;
	class DescriptorPool;
	class DescriptorContainer;
	class DescriptorSet;
	// Shader class:
	class ShaderCompiled;
	// Pipeline class:
	class VertexAttribute;
	class RenderPass;
	class ShaderPipeline;
	// Tick class:
	class RenderTick;
}

// Internal Functions:
export namespace vk {
	std::span<const Layer> getRequestedLayers();
	std::span<const Extension> getRequestedExtensions();
	std::span<const char* const> getRequestedDeviceExtensions();
	VkInstance getVkInstance();
}

// Device class:
namespace vk {
	struct Layer {
		const char* name;
		uint version;
	};
	struct Extension {
		const char* name;
		uint version;
	};
	class PhysicalDevice
	{
	  public:
		PhysicalDevice(
		  VkPhysicalDevice _d, OSRenderSurface* renderSurface = nullptr);
		PhysicalDevice(const PhysicalDevice&) = delete;
		PhysicalDevice(PhysicalDevice&&) noexcept;
		VkPhysicalDevice d;
		VkPhysicalDeviceFeatures2 features10;
		VkPhysicalDeviceVulkan11Features features11;
		VkPhysicalDeviceVulkan12Features features12;
		VkPhysicalDeviceProperties2 properties10;
		VkPhysicalDeviceVulkan11Properties properties11;
		VkPhysicalDeviceVulkan12Properties properties12;
		VkPhysicalDeviceMemoryProperties memProperties;
		std::vector<VkQueueFamilyProperties> queueFamilies;
		std::list<LogicalDevice> devices;
		float rating;
		bool meetRequirements;
		OSRenderSurface* renderOut = nullptr;
		QueueFamilyIndex getQueueFamily(
		  VkQueueFlags requirement, OSRenderSurface* displayOutput = nullptr);
		uint getMemoryTypeIndex(
		  uint bitFilter, Flags<MemoryFeature> feature) const;
		LogicalDevice* makeDevice(
		  VkQueueFlags requirement, OSRenderSurface* renderSurface = nullptr);
		VkPhysicalDevice operator->() { return d; }
		const VkPhysicalDevice operator->() const { return d; }
		bool operator==(const PhysicalDevice& other) const {
			return d == other.d;
		}
		std::partial_ordering operator<=>(const PhysicalDevice& other) const {
			return rating <=> other.rating;
		}
		SwapChainSupport getSwapChainSupport() const;
		~PhysicalDevice();
	};
	class LogicalDevice
	{
	  public:
		LogicalDevice(PhysicalDevice& pd, QueueFamilyIndex queueFamilyIndex);
		LogicalDevice(const LogicalDevice&) = delete;
		LogicalDevice(LogicalDevice&&) noexcept;
		VkDevice d;
		VkQueue queue;
		QueueFamilyIndex queueIndex;
		std::unique_ptr<SwapChain> swapChain;
		std::vector<CommendPool> commendPools;
		void makeSwapChain(uvec2 size = uvec2(-1));
		void remakeSwapChain(uvec2 size = uvec2(-1));
		CommendPool& getCommendPool(CommendPoolType type);
		VkDevice operator->() { return d; }
		const VkDevice operator->() const { return d; }
		~LogicalDevice();
		PhysicalDevice& pd;
	};
	class OSRenderSurface
	{
	  public:
		OSRenderSurface();
		OSRenderSurface(const OSRenderSurface&) = delete;
		OSRenderSurface(OSRenderSurface&&) = delete;
		VkSurfaceKHR surface = nullptr;
		VkSurfaceKHR operator->() { return surface; }
		const VkSurfaceKHR operator->() const { return surface; }
		~OSRenderSurface();
	};
	class SwapChainSupport
	{
	  public:
		SwapChainSupport() = default;
		SwapChainSupport(
		  const PhysicalDevice& pd, const OSRenderSurface& surface);
		VkSurfaceFormatKHR getFormat() const;
		VkPresentModeKHR getPresentMode(bool preferRelaxedVBlank = false) const;
		uvec2 getSwapChainSize(uvec2 preferredSize = uvec2(uint(-1))) const;
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
	class SwapChain
	{
	  public:
		SwapChain(const OSRenderSurface& surface, LogicalDevice& device,
		  uvec2 preferredSize, bool transparentWindow = false);
		SwapChain(const SwapChain&) = delete;
		SwapChain(SwapChain&&) = delete;
		bool rebuildSwapChain(
		  uvec2 preferredSize, bool transparentWindow = false);
		LogicalDevice& d;
		const OSRenderSurface& sf;
		VkSwapchainKHR sc;
		VkSurfaceFormatKHR surfaceFormat;
		uvec2 pixelSize;
		std::vector<VkImage> swapChainImages;
		ImageView getChainImageView(uint index);
		~SwapChain();
	};
}

// Buffer class:
namespace vk {
	class Buffer
	{
		void _setup();

	  public:
		Buffer(LogicalDevice& d, size_t size,
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
		void blockingIndirectWrite(CommendPool& cp, const void* data);
		void blockingIndirectWrite(
		  size_t size, size_t offset, const void* data);
		void blockingIndirectWrite(
		  CommendPool& cp, size_t size, size_t offset, const void* data);
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

// Image class:
namespace vk {
	class FrameBuffer
	{
	  public:
		FrameBuffer(LogicalDevice& device, RenderPass& rp, uvec2 nSize,
		  std::vector<ImageView*> attachments, uint layers = 1);
		FrameBuffer(const FrameBuffer&) = delete;
		FrameBuffer(FrameBuffer&& other) noexcept;
		~FrameBuffer();
		uvec2 size;
		VkFramebuffer fb = nullptr;
		LogicalDevice& d;
	};
	class ImageView
	{
	  public:
		ImageView(LogicalDevice& d, const Image& img);
		ImageView(LogicalDevice& d, VkImageViewCreateInfo createInfo);
		ImageView(const ImageView&) = delete;
		ImageView(ImageView&& other) noexcept;
		ImageView& operator=(const ImageView&) = delete;
		ImageView& operator=(ImageView&&) = default;
		~ImageView();
		VkImageView imgView = nullptr;
		LogicalDevice& d;
	};
	class Image
	{
	  public:
		// Super ctor
		Image(LogicalDevice& d, uvec3 texSize, uint texDimension,
		  VkFormat texFormat,
		  Flags<TextureUseType> texUsage = TextureUseType::None,
		  TextureActiveUseType startingUsage = TextureActiveUseType::Undefined,
		  Flags<MemoryFeature> texMemFeature = MemoryFeature::None,
		  Flags<TextureFeature> texFeature = TextureFeature::None,
		  uint mipLevels = 1, uint layers = 1, uint subsamples = 1);
		Image(const Image&) = delete;
		Image(Image&& other) noexcept;
		Image& operator=(const Image&) = delete;
		Image& operator=(Image&&) = default;
		~Image();

		void* map();
		void* map(size_t size, size_t offset);
		void flush();
		void flush(size_t size, size_t offset);
		void unmap();
		void blockingIndirectWrite(const void* data);
		void blockingIndirectWrite(
		  size_t size, size_t offset, const void* data);
		void directWrite(const void* data);
		void directWrite(size_t size, size_t offset, const void* data);

		void blockingTransformActiveUsage(TextureActiveUseType targetUsage);

		// TODO: Check if below is okay with vkMinAlignment
		size_t mappingMinAlignment;	 // size_t(-1) equals no mapping
		size_t texMemorySize;
		uvec3 size;
		uint dimension;
		uint mipLevels;
		uint layers;
		VkFormat format;
		Flags<MemoryFeature> memFeature;
		Flags<TextureFeature> feature;
		Flags<TextureUseType> usage;
		TextureActiveUseType activeUsage;
		VkImage img = nullptr;
		VkDeviceMemory dm = nullptr;
		void* mappedPtr = nullptr;
		LogicalDevice& d;
	};
}

// Commend class:
namespace vk {
	class CommendPool
	{
	  public:
		CommendPool(LogicalDevice& device, CommendPoolType type);
		CommendPool(const CommendPool&) = delete;
		CommendPool(CommendPool&& other) noexcept;
		~CommendPool();
		VkCommandPool cp = nullptr;
		LogicalDevice& d;
	};
	class CommendBuffer
	{
	  public:
		CommendBuffer(CommendPool& pool);
		CommendBuffer(const CommendBuffer&) = delete;
		CommendBuffer(CommendBuffer&& other) noexcept;
		~CommendBuffer();
		void startRecording(Flags<CommendBufferUsage> usage);
		void cmdBeginRender(
		  const RenderPass& rp, const FrameBuffer& fb, vec4 bgColor);
		void cmdBind(const ShaderPipeline& cb);
		void cmdBind(
		  const VertexBuffer& vb, uint bindingPoint = 0, size_t offset = 0);
		void cmdBind(const IndexBuffer& ib,
		  VkIndexType dataType = _toIndexTpye::val<uint>(), size_t offset = 0);
		void cmdBind(const DescriptorSet& ds,
		  const ShaderPipeline& p);	 // TODO: add multibinding

		void cmdChangeState(Image& target, TextureActiveUseType type,
		  Flags<PipelineStage> srcStage, Flags<MemoryAccess> srcAccess,
		  Flags<PipelineStage> dstStage, Flags<MemoryAccess> dstAccess);

		void cmdDraw(
		  uint vCount, uint iCount = 1, uint vOffset = 0, uint iOffset = 0);
		void cmdDrawIndexed(uint indCount, uint insCount = 1,
		  uint indOffset = 0, uint vertOffset = 0, uint insOffset = 0);
		void cmdCopy(const Buffer& src, Buffer& dst, size_t size,
		  size_t srcOffset = 0, size_t dstOffset = 0);
		void cmdCopy(const Buffer& src, Image& dst, TextureAspect aspect,
		  uvec3 inputSize, uvec3 copySize, ivec3 copyOffset = ivec3(0, 0, 0),
		  size_t inputOffset = 0, uint mipLevel = 0, uint layerOffset = 0,
		  uint layerCount = 1);
		void cmdCopy(Image& src, Buffer& dst, TextureAspect aspect,
		  uvec3 outputSize, uvec3 copySize, ivec3 copyOffset = ivec3(0, 0, 0),
		  size_t outputOffset = 0, uint mipLevel = 0, uint layerOffset = 0,
		  uint layerCount = 1);

		void cmdEndRender();
		void endRecording();
		Fence submit();
		VkCommandBuffer cb = nullptr;
		CommendPool& cp;
	};
}

// Sync class:
namespace vk {
	class Fence
	{
	  public:
		Fence(LogicalDevice& d, bool signaled = false);
		Fence(const Fence&) = delete;
		Fence(Fence&& o) noexcept;
		~Fence();
		void wait();
		VkFence fc;
		LogicalDevice& d;
	};
	class Semaphore
	{
	  public:
		Semaphore(LogicalDevice& d, bool isSet = false);
		Semaphore(const Semaphore&) = delete;
		Semaphore(Semaphore&& o) noexcept;
		~Semaphore();
		VkSemaphore sp;
		LogicalDevice& d;
	};
	class TimelineSemaphore
	{
	  public:
		TimelineSemaphore(LogicalDevice& d, ulint initValue);
		TimelineSemaphore(const TimelineSemaphore&) = delete;
		TimelineSemaphore(TimelineSemaphore&& o) noexcept;
		~TimelineSemaphore();
		VkSemaphore sp;
		LogicalDevice& d;
	};
	struct SyncPoint {
		SyncNumber syncNumber;
		SyncLine* syncLine;
		SyncPoint next();
	};
	struct SyncLine {
		TimelineSemaphore sp;
		SyncNumber topNumber;
		SyncPoint next();
		SyncPoint top();
	};
	inline SyncPoint SyncPoint::next() { return syncLine->next(); }
	inline SyncPoint SyncLine::next() { return SyncPoint{++topNumber, this}; }
	inline SyncPoint SyncLine::top() { return SyncPoint(topNumber, this); }
}

// Descriptor class:
namespace vk {
	struct DescriptorLayoutBinding {
		uint bindPoint;
		DescriptorDataType type;
		uint count;
		Flags<ShaderType> shader = ShaderType::All;
	};
	class DescriptorLayout
	{
	  public:
		struct Size {
			DescriptorDataType type;
			uint n;
		};
		inline DescriptorLayout(LogicalDevice& d,
		  ViewableWith<const DescriptorLayoutBinding&> auto bindings);
		DescriptorLayout(const DescriptorLayout&) = delete;
		DescriptorLayout(DescriptorLayout&& other) noexcept;
		~DescriptorLayout();
		std::span<const Size> getSizes() const;
		VkDescriptorSetLayout dsl;
		LogicalDevice& d;

	  private:
		std::vector<Size> _size;
	};
	class DescriptorPool
	{
	  public:
		inline DescriptorPool(LogicalDevice& d, uint setCount,
		  ViewableWith<const DescriptorLayout::Size&> auto size,
		  Flags<DescriptorPoolType> requirement = DescriptorPoolType::None);
		DescriptorPool(const DescriptorPool&) = delete;
		DescriptorPool(DescriptorPool&& other) noexcept;
		~DescriptorPool();
		DescriptorSet allocNewSet(const DescriptorLayout& ul);
		std::vector<DescriptorSet> allocNewSet(
		  ViewableWith<const DescriptorLayout&> auto uls);
		Flags<vk::DescriptorPoolType> settings;
		VkDescriptorPool dp;
		LogicalDevice& d;
	};
	// Auto-Fitting DescriptorPool
	class DescriptorContainer: protected DescriptorPool
	{
	  public:
		DescriptorContainer(LogicalDevice& d, const DescriptorLayout& ul,
		  uint setCount,
		  Flags<DescriptorPoolType> requirement = DescriptorPoolType::None);
		DescriptorContainer(const DescriptorContainer&) = delete;
		DescriptorContainer(DescriptorContainer&& other) noexcept;
		~DescriptorContainer();
		operator const DescriptorPool&() const;
		DescriptorSet allocNewSet();
		std::vector<DescriptorSet> allocNewSet(uint count);
		const DescriptorLayout& ul;
	};
	template<typename T>
	concept WriteCmd = requires(T t) {
		{ t.data } -> ViewableWith<const DescriptorSet::WriteData&>;
		// requires std::same_as<T,
		//  DescriptorSet::template CmdWrite<decltype(t.data)>>;
	};
	class DescriptorSet
	{
	  public:
		struct WriteData {
			struct BufferType {
				const Buffer* buffer;
				size_t offset = 0;
				size_t length = VK_WHOLE_SIZE;
			};
			struct ImageType {
				const ImageView* imageView;
				VkSampler sampler;
				VkImageLayout imageLayout;
			};
			union {
				BufferType asBuffer;
				ImageType asImage;
			};
			WriteData(
			  const Buffer* buf, size_t off = 0, size_t len = VK_WHOLE_SIZE) {
				asBuffer.buffer = buf;
				asBuffer.offset = off;
				asBuffer.length = len;
			}
			WriteData(
			  const ImageView* img, VkSampler s, VkImageLayout imgLayout) {
				asImage.imageView = img;
				asImage.sampler = s;
				asImage.imageLayout = imgLayout;
			}
		};
		struct WriteParam {
			uint bindPoint;
			DescriptorDataType type;
			uint count;
			uint offset;
		};
		template<ViewableWith<const WriteData&> View> struct CmdWrite {
			DescriptorSet* target;
			WriteParam paramater;
			View data;
		};

		DescriptorSet(DescriptorPool& dp, const DescriptorLayout& ul);
		DescriptorSet(DescriptorPool& dp, VkDescriptorSet ds);
		DescriptorSet(const DescriptorSet&) = delete;
		DescriptorSet(DescriptorSet&& other) noexcept;
		~DescriptorSet();
		VkDescriptorSet ds;
		DescriptorPool& dp;
		static inline std::vector<DescriptorSet> makeBatch(
		  DescriptorPool& dp, ViewableWith<const DescriptorLayout&> auto uls);
		template<typename View>
		requires requires(View v) {
			requires Viewable<View>;
			{ *v.begin() } -> WriteCmd;
		}
		static inline void blockingWrite(LogicalDevice& d, View v);
		inline void blockingWrite(uint bindPoint, DescriptorDataType type,
		  uint count, uint offset, ViewableWith<const WriteData&> auto data);
	};

	inline VkDescriptorSetLayout _toDsl(const DescriptorLayout& dl) {
		return dl.dsl;
	}
	inline VkDescriptorPoolSize _toDps(const DescriptorLayout::Size& dl) {
		return VkDescriptorPoolSize{
		  .type = static_cast<VkDescriptorType>(dl.type),
		  .descriptorCount = dl.n};
	}

	inline DescriptorLayout::DescriptorLayout(LogicalDevice& d,
	  ViewableWith<const DescriptorLayoutBinding&> auto bindings):
	  d(d) {
		std::vector<VkDescriptorSetLayoutBinding> b;
		std::map<VkDescriptorType, uint> s;
		b.resize(bindings.size());
		uint i = 0;
		for (auto& bind : bindings) {
			b[i].binding = bind.bindPoint;
			b[i].descriptorCount = bind.count;
			b[i].descriptorType = static_cast<VkDescriptorType>(bind.type);
			b[i].stageFlags = (VkShaderStageFlags)bind.shader;
			s[b[i].descriptorType]++;
			i++;
		}
		VkDescriptorSetLayoutCreateInfo dInfo{};
		dInfo.bindingCount = i;
		dInfo.pBindings = b.data();
		dInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		vkCreateDescriptorSetLayout(d.d, &dInfo, nullptr, &dsl);
		_size.reserve(s.size());
		for (auto p : s) {
			_size.push_back(
			  Size{static_cast<DescriptorDataType>(p.first), p.second});
		}
	}

	inline vk::DescriptorPool::DescriptorPool(LogicalDevice& d, uint setCount,
	  ViewableWith<const DescriptorLayout::Size&> auto size,
	  Flags<DescriptorPoolType> requirement):
	  d(d) {
		settings = requirement;
		auto tView =
		  View(size.begin(), size.end()).pipeWith<decltype(&_toDps), &_toDps>();
		std::vector<VkDescriptorPoolSize> dps{tView.begin(), tView.end()};
		VkDescriptorPoolCreateInfo cInfo{};
		cInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		cInfo.flags = settings;
		cInfo.maxSets = setCount;
		cInfo.poolSizeCount = dps.size();
		cInfo.pPoolSizes = dps.data();
		vkCreateDescriptorPool(d.d, &cInfo, nullptr, &dp);
	}

	inline std::vector<DescriptorSet> DescriptorPool::allocNewSet(
	  ViewableWith<const DescriptorLayout&> auto uls) {
		return DescriptorSet::makeBatch(*this, uls);
	}

	inline std::vector<DescriptorSet> DescriptorSet::makeBatch(
	  DescriptorPool& dp, ViewableWith<const DescriptorLayout&> auto uls) {
		auto tvw =
		  View{uls.begin(), uls.end()}.pipeWith<decltype(&_toDsl), &_toDsl>();
		std::vector<VkDescriptorSetLayout> dsls{tvw.begin(), tvw.end()};
		std::vector<VkDescriptorSet> dss{};
		dss.resize(dsls.size());

		VkDescriptorSetAllocateInfo aInfo{};
		aInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		aInfo.descriptorPool = dp.dp;
		aInfo.descriptorSetCount = dsls.size();
		aInfo.pSetLayouts = dsls.data();
		vkAllocateDescriptorSets(dp.d.d, &aInfo, dss.data());

		std::vector<DescriptorSet> result{};
		result.reserve(dss.size());
		for (auto ptr : dss) result.emplace_back(dp, ptr);
		return result;
	}

	template<typename View>
	requires requires(View v) {
		requires Viewable<View>;
		{ *v.begin() } -> WriteCmd;
	}
	inline void vk::DescriptorSet::blockingWrite(LogicalDevice& d, View v) {
		using WriteCmdT = decltype(*v.begin());
		std::vector<VkWriteDescriptorSet> wds{};
		std::list<std::vector<VkDescriptorImageInfo>> iInfol{};
		std::list<std::vector<VkDescriptorBufferInfo>> bInfol{};
		for (auto cmd : v) {
			auto cmdInfo = VkWriteDescriptorSet{
			  .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			  .pNext = nullptr,
			  .dstSet = cmd.target->ds,
			  .dstBinding = cmd.paramater.bindPoint,
			  .dstArrayElement = cmd.paramater.offset,
			  .descriptorCount = cmd.paramater.count,
			  .descriptorType = (VkDescriptorType)cmd.paramater.type,
			};
			switch (cmd.paramater.type) {
			case DescriptorDataType::UniformBuffer:
			case DescriptorDataType::StorageBuffer: {
				auto& bInfo = bInfol.emplace_back();
				bInfo.reserve(cmdInfo.descriptorCount);
				for (WriteData d : cmd.data) {
					bInfo.push_back(VkDescriptorBufferInfo{
					  .buffer = d.asBuffer.buffer->b,
					  .offset = d.asBuffer.offset,
					  .range = d.asBuffer.length,
					});
				}
				if (bInfo.size() != cmdInfo.descriptorCount)
					throw "VulkanInvalidCmdWriteNumOfData";
				cmdInfo.pImageInfo = nullptr;
				cmdInfo.pBufferInfo = bInfo.data();
				cmdInfo.pTexelBufferView = nullptr;
			} break;
			case DescriptorDataType::Sampler: throw "NotImplemented"; break;
			}
			wds.push_back(cmdInfo);
		}
		// vkDeviceWaitIdle(d.d); //Unsure if needed
		vkUpdateDescriptorSets(d.d, wds.size(), wds.data(), 0, nullptr);
		// vkDeviceWaitIdle(d.d); //Unsure if needed
	}
	inline void vk::DescriptorSet::blockingWrite(uint bindPoint,
	  DescriptorDataType type, uint count, uint offset,
	  ViewableWith<const WriteData&> auto data) {
		using DataViewT = decltype(data);
		blockingWrite(dp.d,
		  std::array<CmdWrite<decltype(data)>, 1>{CmdWrite<decltype(data)>{
			.target = this,
			.paramater =
			  WriteParam{
				.bindPoint = bindPoint,
				.type = type,
				.count = count,
				.offset = offset,
			  },
			.data = data,
		  }});
	}

}

// Shader class:
namespace vk {
	class ShaderCompiled
	{
	  public:
		ShaderCompiled(LogicalDevice& d, ShaderType shaderType,
		  const std::string_view& file_name);
		ShaderCompiled(const ShaderCompiled&) = delete;
		ShaderCompiled(ShaderCompiled&& other) noexcept;
		~ShaderCompiled();
		VkPipelineShaderStageCreateInfo getCreateInfo() const;
		ShaderType shaderType;
		VkShaderModule sm;
		LogicalDevice& d;
	};
}

// Pipeline class:
namespace vk {
	class VertexAttribute
	{
	  public:
		VertexAttribute();
		void addAttribute(uint bindingPoint, uint size, VkFormat format);
		template<typename T> void addAttributeByType(uint bindingPoint = 0) {
			addAttribute(bindingPoint, sizeof(T), _toFormat::val<T>());
		}
		void addBindingPoint(uint stride = uint(-1),
		  BufferInputRate rate = BufferInputRate::PerVertex);
		std::vector<VkVertexInputAttributeDescription> attributes;
		std::vector<VkVertexInputBindingDescription> bindingPoints;
		uint strideSize = 0;
		VkPipelineVertexInputStateCreateInfo getStructForPipeline();
	};
	class RenderPass
	{
	  public:
		RenderPass(LogicalDevice& d);
		RenderPass(const RenderPass& d) = delete;
		RenderPass(RenderPass&& d) noexcept;
		~RenderPass();
		void complete();
		VkRenderPass rp = nullptr;
		std::vector<VkAttachmentDescription> attachmentTypes;
		std::vector<std::vector<uint>> subpasses;
		std::vector<VkSubpassDependency> subpassDependencies;
		LogicalDevice& d;
	};
	class ShaderPipeline
	{
	  public:
		ShaderPipeline(LogicalDevice& device);
		ShaderPipeline(const ShaderPipeline&) = delete;
		ShaderPipeline(ShaderPipeline&& other) noexcept;
		~ShaderPipeline();
		void bind(const DescriptorLayout& dsl);
		void complete(std::vector<const ShaderCompiled*> shaderModules,
		  VertexAttribute& va, VkViewport viewport,
		  const RenderPass& renderPass);
		VkPipeline p = nullptr;
		VkPipelineLayout pl = nullptr;
		LogicalDevice& d;

	  private:
		std::vector<VkDescriptorSetLayout> _dsl;
	};
}

// Tick class:
namespace vk {
	constexpr size_t RENDERTICK_INITIAL_MBR_SIZE = 512;
	class RenderTick
	{
	  public:
		RenderTick(LogicalDevice& d);
		RenderTick(const RenderTick&) = delete;
		RenderTick(RenderTick&&) = delete;
		SyncPoint makeSyncLine(SyncNumber initialValue = 0);
		void addCmdStage(CommendBuffer& cb,
		  const std::vector<SyncPoint>& signalTo,
		  const std::vector<SyncPoint>& waitFor,
		  const std::vector<VkPipelineStageFlags>& waitType);
		SyncPoint getTopSyncPoint();
		void addSyncPointToLayer();
		void pushSyncPointStack(SyncPoint syncPoint);
		SyncPoint popSyncPointStack();	// return layerEndingSyncPoint
		void send();
		void notifyOutdated();
		bool isOutdated() const { return outdated; }
		bool isStarted() const { return _waitingForFence; }
		uint getImageIndex() const { return imageIndex; }
		bool isCompleted() const;
		bool waitForCompletion(
		  ulint timeout = -1) const;  // return false for timeout
		Buffer& makeStagingBuffer(size_t size);
		CommendBuffer& makeSingleUseCommendBuffer(CommendPool& cp);
		void forceKill();
		~RenderTick();	// Will wait for completion
		LogicalDevice& d;

	  private:
		bool _waitingForFence;
		Fence _completionFence;
		Semaphore _acquireSemaphore;
		Semaphore _presentSemaphore;
		util::MBRPool _submitPools;
		std::list<SyncLine> _syncLines;
		std::stack<SyncPoint> _syncPointStack;
		std::vector<VkSubmitInfo> _cmdStages;
		std::list<Buffer> _stagingBuffers;
		std::list<CommendBuffer> _sigleUseCommendBuffer;
		uint imageIndex;
		bool outdated;
	};
}

module;
#include "Marco.h"
#ifdef USE_VULKAN
#	pragma warning(disable : 26812)
#	include <vulkan/vulkan.h>
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
	// Buffer
	inline Buffer::Buffer(LogicalDevice& device, size_t s,
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
		if (feature.has(MemoryFeature::IndirectCopyable)) {
			usage.set(BufferUseType::TransferSrc);
		}
		if (feature.has(MemoryFeature::IndirectWritable)) {
			usage.set(BufferUseType::TransferDst);
		}
		_setup();
	}
	inline Buffer::Buffer(Buffer&& other) noexcept: d(other.d) {
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
		ImageView(LogicalDevice& d, VkImageViewCreateInfo createInfo);
		ImageView(const ImageView&) = delete;
		ImageView(ImageView&& other) noexcept;
		ImageView& operator=(const ImageView&) = delete;
		ImageView& operator=(ImageView&&) = default;
		~ImageView();
		VkImageView imgView;
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
		void cmdDraw(
		  uint vCount, uint iCount = 1, uint vOffset = 0, uint iOffset = 0);
		void cmdDrawIndexed(uint indCount, uint insCount = 1,
		  uint indOffset = 0, uint vertOffset = 0, uint insOffset = 0);
		void cmdCopyBuffer(const Buffer& src, Buffer& dst, size_t size,
		  size_t srcOffset = 0, size_t dstOffset = 0);
		void cmdEndRender();
		void endRecording();
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
		DescriptorLayout(LogicalDevice& d,
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
		DescriptorPool(LogicalDevice& d,
		  std::span<const DescriptorLayout::Size> size, uint setCount,
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
	class DescriptorContainer: DescriptorPool
	{
	  public:
		DescriptorContainer(LogicalDevice& d, const DescriptorLayout& ul,
		  uint setCount,
		  Flags<DescriptorPoolType> requirement = DescriptorPoolType::None);
		DescriptorContainer(const DescriptorContainer&) = delete;
		DescriptorContainer(DescriptorContainer&& other) noexcept;
		~DescriptorContainer();
		DescriptorSet allocNewSet();
		std::vector<DescriptorSet> allocNewSet(uint count);
		const DescriptorLayout& ul;
	};
	class DescriptorSet
	{
	  public:
		DescriptorSet(DescriptorPool& dp, const DescriptorLayout& ul);
		static std::vector<DescriptorSet> makeBatch(
		  DescriptorPool& dp,
		  ViewableWith<const DescriptorLayout&> auto uls);
		DescriptorSet(DescriptorPool& dp, VkDescriptorSet ds);
		DescriptorSet(const DescriptorSet&) = delete;
		DescriptorSet(DescriptorSet&& other) noexcept;
		~DescriptorSet();
		VkDescriptorSet ds;
		DescriptorPool& dp;
	};

	inline VkDescriptorSetLayout _toDsl(const DescriptorLayout& dl) {
		return dl.dsl;
	}

	DescriptorLayout::DescriptorLayout(LogicalDevice& d,
	  ViewableWith<const DescriptorLayoutBinding&> auto bindings) {
		std::vector<VkDescriptorSetLayoutBinding> b;
		std::map<VkDescriptorType, uint> s;
		b.resize(bindings.size());
		uint i = 0;
		for (auto& bind : bindings) {
			b[i].binding = bind.bindPoint;
			b[i].descriptorCount = bind.count;
			b[i].descriptorType = static_cast<VkDescriptorType>(bind.type);
			b[i].stageFlags = bind.shader;
			s[b[i].descriptorType]++;
			i++;
		}
		VkDescriptorSetLayoutCreateInfo dInfo{};
		dInfo.bindingCount = i;
		dInfo.pBindings = b.data();
		dInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		vkCreateDescriptorSetLayout(d.d, &dInfo, nullptr, &dsl);
		_size.reserve(s.size());
		for (auto p : s) { _size.emplace_back(p.first, p.second); }
	}
	std::vector<DescriptorSet> vk::DescriptorPool::allocNewSet(
	  ViewableWith<const DescriptorLayout&> auto uls) {
		return DescriptorSet::makeBatch(*this, uls);
	}
	std::vector<DescriptorSet> DescriptorSet::makeBatch(
	  DescriptorPool& dp,
	  ViewableWith<const DescriptorLayout&> auto uls) {
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
		void complete(std::vector<const ShaderCompiled*> shaderModules,
		  VertexAttribute& va, VkViewport viewport,
		  const RenderPass& renderPass);
		VkPipeline p = nullptr;
		VkPipelineLayout pl = nullptr;
		LogicalDevice& d;
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

#else
export module Vulkan: Internal;
#endif
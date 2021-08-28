export module Vulkan: Descriptor;
export import: Internal;
import: Device;
import: Enum;
import Define;
import Util;
import "VulkanExtModule.h";

// Descriptor class:
export namespace vk {
	struct DescriptorLayoutBinding {
		uint bindPoint;
		DescriptorDataType type;
		uint count;
		Flags<ShaderType> shader = ShaderType::All;
	};
	class DescriptorLayout
	{
		void _ctor(std::span<const DescriptorLayoutBinding> bindings);

	  public:
		struct Size {
			DescriptorDataType type;
			uint n;
		};
		DescriptorLayout(
		  LogicalDevice& d, std::span<const DescriptorLayoutBinding> bindings);
		DescriptorLayout(LogicalDevice& d,
		  std::initializer_list<DescriptorLayoutBinding> bindings);

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
		void _ctor(uint setCount, std::span<const DescriptorLayout::Size> size,
		  Flags<DescriptorPoolType> requirement);

	  public:
		DescriptorPool(LogicalDevice& d, uint setCount,
		  std::span<const DescriptorLayout::Size> size,
		  Flags<DescriptorPoolType> requirement = DescriptorPoolType::None);
		DescriptorPool(LogicalDevice& d, uint setCount,
		  std::initializer_list<DescriptorLayout::Size> size,
		  Flags<DescriptorPoolType> requirement = DescriptorPoolType::None);

		DescriptorPool(const DescriptorPool&) = delete;
		DescriptorPool(DescriptorPool&& other) noexcept;
		~DescriptorPool();
		DescriptorSet allocNewSet(const DescriptorLayout& ul);
		std::vector<DescriptorSet> allocNewSets(
		  std::span<Ref<const DescriptorLayout>> uls);
		std::vector<DescriptorSet> allocNewSets(
		  std::initializer_list<Ref<const DescriptorLayout>> uls);

		Flags<vk::DescriptorPoolType> settings;
		VkDescriptorPool dp;
		LogicalDevice& d;
	};
	// Auto-Fitting DescriptorPool
	class DescriptorContainer: protected DescriptorPool
	{
	  public:
		DescriptorContainer(vk::LogicalDevice& d, const DescriptorLayout& ul,
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
	class DescriptorSet
	{
	  public:
		struct WriteData {
			struct BufferType {
				const Buffer* buffer;
				size_t offset = 0;
				size_t length = VK_WHOLE_SIZE;
			};
			struct ImageAndSamplerType {
				const ImageView* imageView;
				const ImageSampler* sampler;
				TextureActiveUseType imageActiveUsage;
			};
			union {
				BufferType asBuffer;
				ImageAndSamplerType asImage;
			};
			WriteData(
			  const Buffer* buf, size_t off = 0, size_t len = VK_WHOLE_SIZE) {
				asBuffer.buffer = buf;
				asBuffer.offset = off;
				asBuffer.length = len;
			}
			WriteData(const ImageView* imgView, const ImageSampler* s,
			  TextureActiveUseType imgActiveUsage) {
				asImage.imageView = imgView;
				asImage.sampler = s;
				asImage.imageActiveUsage = imgActiveUsage;
			}
			WriteData(
			  const ImageView* imgView, TextureActiveUseType imgActiveUsage):
			  WriteData(imgView, nullptr, imgActiveUsage) {}
			WriteData(const ImageSampler* s):
			  WriteData(nullptr, s, TextureActiveUseType::Undefined) {}
		};

		struct CmdWriteInfo {
			DescriptorSet* target;
			uint bindPoint;
			DescriptorDataType type;
			uint count;
			uint offset;
			std::initializer_list<WriteData> data;
		};

		DescriptorSet(DescriptorPool& dp, const DescriptorLayout& ul);
		DescriptorSet(DescriptorPool& dp, VkDescriptorSet ds);
		DescriptorSet(const DescriptorSet&) = delete;
		DescriptorSet(DescriptorSet&& other) noexcept;
		~DescriptorSet();
		VkDescriptorSet ds;
		DescriptorPool& dp;
		static std::vector<DescriptorSet> makeBatch(
		  DescriptorPool& dp, std::span<const Ref<const DescriptorLayout>> uls);
		static std::vector<DescriptorSet> makeBatch(DescriptorPool& dp,
		  std::initializer_list<Ref<const DescriptorLayout>> uls);

		void blockingWrite(uint bindPoint, DescriptorDataType type, uint count,
		  uint offset, std::initializer_list<WriteData> data);

		static void blockingWrite(
		  LogicalDevice& d, std::initializer_list<CmdWriteInfo> cmds);
		static void blockingWrite(
		  LogicalDevice& d, std::span<const CmdWriteInfo> cmds);
	};
}

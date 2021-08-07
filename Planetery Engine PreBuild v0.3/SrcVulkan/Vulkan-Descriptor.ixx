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
	  public:
		struct Size {
			DescriptorDataType type;
			uint n;
		};
		inline DescriptorLayout(vk::LogicalDevice& d,
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
		inline DescriptorPool(vk::LogicalDevice& d, uint setCount,
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
			case DescriptorDataType::Image:
			case DescriptorDataType::Sampler:
			case DescriptorDataType::ImageAndSampler:
				auto& iInfo = iInfol.emplace_back();
				iInfo.reserve(cmdInfo.descriptorCount);
				for (WriteData d : cmd.data) {
					iInfo.push_back(VkDescriptorImageInfo{
					  .sampler = d.asImage.sampler != nullptr
								 ? d.asImage.sampler->smp
								 : nullptr,
					  .imageView = d.asImage.imageView != nullptr
								   ? d.asImage.imageView->imgView
								   : nullptr,
					  .imageLayout = (VkImageLayout)d.asImage.imageActiveUsage,
					});
				}
				if (iInfo.size() != cmdInfo.descriptorCount)
					throw "VulkanInvalidCmdWriteNumOfData";
				cmdInfo.pImageInfo = iInfo.data();
				cmdInfo.pBufferInfo = nullptr;
				cmdInfo.pTexelBufferView = nullptr;
				break;
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

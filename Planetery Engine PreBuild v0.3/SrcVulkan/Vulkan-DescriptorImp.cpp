module Vulkan: DescriptorImp;
import: Device;
import: Descriptor;
import: Enum;
import: Buffer;
import: Image;
import std.core;
import Define;
import Logger;

// DescriptorLayout
namespace vk {
	void DescriptorLayout::_ctor(
	  std::span<const DescriptorLayoutBinding> bindings) {
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

	vk::DescriptorLayout::DescriptorLayout(
	  LogicalDevice& d, std::span<const DescriptorLayoutBinding> bindings):
	  d(d) {
		_ctor(bindings);
	}
	vk::DescriptorLayout::DescriptorLayout(LogicalDevice& d,
	  std::initializer_list<DescriptorLayoutBinding> bindings):
	  d(d) {
		_ctor(asSpan(bindings));
	}

	DescriptorLayout::DescriptorLayout(DescriptorLayout&& o) noexcept: d(o.d) {
		dsl = o.dsl;
		o.dsl = nullptr;
	}
	DescriptorLayout::~DescriptorLayout() {
		if (dsl != nullptr) vkDestroyDescriptorSetLayout(d.d, dsl, nullptr);
	}
	std::span<const DescriptorLayout::Size> DescriptorLayout::getSizes() const {
		return std::span<const Size>(_size);
	}

}

// DescriptorPool
namespace vk {
	void vk::DescriptorPool::_ctor(uint setCount,
	  std::span<const DescriptorLayout::Size> size,
	  Flags<DescriptorPoolType> requirement) {
		settings = requirement;
		VkDescriptorPoolCreateInfo cInfo{};
		cInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		cInfo.flags = settings;
		cInfo.maxSets = setCount;
		cInfo.poolSizeCount = size.size();
		cInfo.pPoolSizes = (const VkDescriptorPoolSize*)size.data();
		vkCreateDescriptorPool(d.d, &cInfo, nullptr, &dp);
	}

	vk::DescriptorPool::DescriptorPool(LogicalDevice& d, uint setCount,
	  std::span<const DescriptorLayout::Size> size,
	  Flags<DescriptorPoolType> requirement):
	  d(d) {
		_ctor(setCount, size, requirement);
	}

	vk::DescriptorPool::DescriptorPool(LogicalDevice& d, uint setCount,
	  std::initializer_list<DescriptorLayout::Size> size,
	  Flags<DescriptorPoolType> requirement):
	  d(d) {
		_ctor(setCount, asSpan(size), requirement);
	}

	// DescriptorPool::DescriptorPool(...) ctor is a template function
	DescriptorPool::DescriptorPool(DescriptorPool&& o) noexcept: d(o.d) {
		dp = o.dp;
		o.dp = nullptr;
		settings = o.settings;
	}
	DescriptorPool::~DescriptorPool() {
		if (dp != nullptr) vkDestroyDescriptorPool(d.d, dp, nullptr);
	}

	DescriptorSet DescriptorPool::allocNewSet(const DescriptorLayout& ul) {
		return DescriptorSet(*this, ul);
	}
	std::vector<DescriptorSet> vk::DescriptorPool::allocNewSets(
	  std::span<Ref<const DescriptorLayout>> uls) {
		return DescriptorSet::makeBatch(*this, uls);
	}
	std::vector<DescriptorSet> vk::DescriptorPool::allocNewSets(
	  std::initializer_list<Ref<const DescriptorLayout>> uls) {
		return DescriptorSet::makeBatch(*this, uls);
	}

}

// DescriptorContainer
namespace vk {
	DescriptorContainer::DescriptorContainer(LogicalDevice& device,
	  const DescriptorLayout& refSet, uint setCount,
	  Flags<DescriptorPoolType> requirement):
	  DescriptorPool(device, setCount, refSet.getSizes(), requirement),
	  ul(refSet) {}
	DescriptorContainer::DescriptorContainer(DescriptorContainer&& o) noexcept:
	  DescriptorPool(std::move(o)), ul(o.ul) {}
	DescriptorContainer::~DescriptorContainer() = default;
	DescriptorContainer::operator const DescriptorPool&() const {
		return *this;
	}
	DescriptorSet DescriptorContainer::allocNewSet() {
		return DescriptorSet(*this, ul);
	}
	std::vector<DescriptorSet> DescriptorContainer::allocNewSet(uint count) {
		std::vector<Ref<const DescriptorLayout>> vec{count, ul};
		return DescriptorSet::makeBatch(*this, vec);
	}
}

// DescriptorSet
namespace vk {
	DescriptorSet::DescriptorSet(
	  DescriptorPool& dp, const DescriptorLayout& dl):
	  dp(dp) {
		VkDescriptorSetAllocateInfo aInfo{};
		aInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		aInfo.descriptorPool = dp.dp;
		aInfo.descriptorSetCount = 1;
		aInfo.pSetLayouts = &dl.dsl;
		if (vkAllocateDescriptorSets(dp.d.d, &aInfo, &ds) != VK_SUCCESS) {
			throw "VulkanDescriptorSetCreateFailure";
		}
		_resetable = dp.settings.has(DescriptorPoolType::Resetable);
	}
	DescriptorSet::DescriptorSet(DescriptorPool& dp, VkDescriptorSet _ds):
	  dp(dp) {
		ds = _ds;
	}
	DescriptorSet::DescriptorSet(DescriptorSet&& o) noexcept: dp(o.dp) {
		ds = o.ds;
		o.ds = nullptr;
	}
	DescriptorSet::~DescriptorSet() {
		// Check if clean up is required as call to free may not be needed
		if (ds != nullptr && _resetable) {
			vkFreeDescriptorSets(dp.d.d, dp.dp, 1, &ds);
		}
	}

	std::vector<DescriptorSet> DescriptorSet::makeBatch(
	  DescriptorPool& dp, std::span<const Ref<const DescriptorLayout>> uls) {
		std::vector<VkDescriptorSetLayout> dsls{};
		dsls.reserve(uls.size());
		for (auto& ulRef : uls) dsls.push_back(ulRef.get().dsl);
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
	std::vector<DescriptorSet> DescriptorSet::makeBatch(DescriptorPool& dp,
	  std::initializer_list<Ref<const DescriptorLayout>> uls) {
		return makeBatch(dp, asSpan(uls));
	}

	void DescriptorSet::blockingWrite(uint bindPoint, DescriptorDataType type,
	  uint count, uint offset, std::initializer_list<WriteData> data) {
		blockingWrite(dp.d, {CmdWriteInfo{
							  .target = this,
							  .bindPoint = bindPoint,
							  .type = type,
							  .count = count,
							  .offset = offset,
							  .data = data,
							}});
	}


	void DescriptorSet::blockingWrite(
	  LogicalDevice& d, std::initializer_list<CmdWriteInfo> cmds) {
		blockingWrite(d, asSpan(cmds));
	}

	void DescriptorSet::blockingWrite(
	  LogicalDevice& d, std::span<const CmdWriteInfo> cmds) {
		std::vector<VkWriteDescriptorSet> wds{};
		std::list<std::vector<VkDescriptorImageInfo>> iInfol{};
		std::list<std::vector<VkDescriptorBufferInfo>> bInfol{};
		for (auto& cmd : cmds) {
			auto cmdInfo = VkWriteDescriptorSet{
			  .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			  .pNext = nullptr,
			  .dstSet = cmd.target->ds,
			  .dstBinding = cmd.bindPoint,
			  .dstArrayElement = cmd.offset,
			  .descriptorCount = cmd.count,
			  .descriptorType = (VkDescriptorType)cmd.type,
			};
			switch (cmd.type) {
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
}

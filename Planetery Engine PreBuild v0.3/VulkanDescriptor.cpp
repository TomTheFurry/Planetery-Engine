module Vulkan;
import: Internal;
import: Enum;
import std.core;
import Define;
import Logger;

// DescriptorLayout
namespace vk {
	// DescriptorLayout::DescriptorLayout(...) ctor is a template function
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
		auto v = util::makeRepeatingView(ul, count);
		static_assert(Viewable<decltype(v)>);
		return DescriptorSet::makeBatch(*this, v);
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
		vkAllocateDescriptorSets(dp.d.d, &aInfo, &ds);
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
		//TODO: Keep track of whether clean up is required using only itself to fix static uninit error
		if (ds != nullptr && dp.settings.has(DescriptorPoolType::Resetable)) {
			vkFreeDescriptorSets(dp.d.d, dp.dp, 1, &ds);
		}
	}
}

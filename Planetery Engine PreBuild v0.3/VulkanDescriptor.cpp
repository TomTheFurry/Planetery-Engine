module;
#include "Marco.h"
#ifdef USE_VULKAN
#	pragma warning(disable : 26812)
#	include <vulkan/vulkan.h>
#	include <assert.h>
module Vulkan;
import: Internal;
import: Enum;
import std.core;
import Define;
import Logger;
using namespace vk;

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

DescriptorPool::DescriptorPool(LogicalDevice& device,
  std::span<const DescriptorLayout::Size> size, uint setCount,
  Flags<UniformPoolType> requirement):
  d(device) {}

DescriptorSet DescriptorPool::allocNewSet(const DescriptorLayout& ul) {
	return DescriptorSet(*this, ul);
}
DescriptorPool::~DescriptorPool() {
	if (dp != nullptr) vkDestroyDescriptorPool(d.d, dp, nullptr);
}


DescriptorContainer::DescriptorContainer(LogicalDevice& device,
  const DescriptorLayout& refSet, uint setCount,
  Flags<UniformPoolType> requirement):
  DescriptorPool(device, refSet.getSizes(), setCount, requirement),
  ul(refSet) {}

DescriptorSet DescriptorContainer::allocNewSet() { return DescriptorSet(*this, ul); }

std::vector<DescriptorSet> DescriptorContainer::allocNewSet(uint count) {
	auto v = util::makeRepeatingView(ul, count);
	static_assert(Viewable<decltype(v)>);
	return DescriptorSet::makeBatch(v);
}

DescriptorSet::DescriptorSet(DescriptorPool& up, const DescriptorLayout& ul): d(up.d) {
	// TODO
}
DescriptorSet::~DescriptorSet() {
	// if (ds != nullptr) vkDestroyDescriptorSet(d.d, ds, nullptr);
}

#else
module Vulkan;
#endif
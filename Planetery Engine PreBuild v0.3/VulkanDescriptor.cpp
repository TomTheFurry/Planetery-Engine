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

// UniformLayout::UniformLayout(...) ctor is a template function

UniformLayout::UniformLayout(UniformLayout&& o) noexcept: d(o.d) {
	dsl = o.dsl;
	o.dsl = nullptr;
}
UniformLayout::~UniformLayout() {
	if (dsl != nullptr) vkDestroyDescriptorSetLayout(d.d, dsl, nullptr);
}

std::span<const UniformLayout::Size> UniformLayout::getSizes() const {
	return std::span<const Size>(_size);
}

UniformPool::UniformPool(LogicalDevice& device,
  std::span<const UniformLayout::Size> size, uint setCount,
  Flags<UniformPoolType> requirement):
  d(device) {}

UniformSet UniformPool::allocNewSet(const UniformLayout& ul) {
	return UniformSet(*this, ul);
}
UniformPool::~UniformPool() {
	if (dp != nullptr) vkDestroyDescriptorPool(d.d, dp, nullptr);
}


LinkedUniformPool::LinkedUniformPool(LogicalDevice& device,
  const UniformLayout& refSet, uint setCount,
  Flags<UniformPoolType> requirement):
  UniformPool(device, refSet.getSizes(), setCount, requirement),
  ul(refSet) {}

UniformSet LinkedUniformPool::allocNewSet() { return UniformSet(*this, ul); }

std::vector<UniformSet> LinkedUniformPool::allocNewSet(uint count) {
	auto v = util::makeRepeatingView(ul, count);
	static_assert(Viewable<decltype(v)>);
	return UniformSet::makeBatch(v);
}

UniformSet::UniformSet(UniformPool& up, const UniformLayout& ul): d(up.d) {
	// TODO
}
UniformSet::~UniformSet() {
	// if (ds != nullptr) vkDestroyDescriptorSet(d.d, ds, nullptr);
}

#else
module Vulkan;
#endif
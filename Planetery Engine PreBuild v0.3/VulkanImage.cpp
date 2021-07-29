module;
#include "Marco.h"
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

ImageView::ImageView(LogicalDevice& d, VkImageViewCreateInfo createInfo): d(d) {
	if (vkCreateImageView(d.d, &createInfo, nullptr, &imgView) != VK_SUCCESS) {
		logger("Vulkan failed to make Image View form VkImage!\n");
		throw "VulkanCreateImageViewFailure";
	}
}
ImageView::ImageView(ImageView&& other) noexcept: d(other.d) {
	imgView = other.imgView;
	other.imgView = nullptr;
}
ImageView::~ImageView() {
	if (imgView != nullptr) vkDestroyImageView(d.d, imgView, nullptr);
}

FrameBuffer::FrameBuffer(LogicalDevice& device, RenderPass& rp, uvec2 nSize,
  std::vector<ImageView*> attachments, uint layers):
  d(device) {
	size = nSize;
	std::vector<VkImageView> att;
	att.reserve(attachments.size());
	for (auto& iv : attachments) att.push_back(iv->imgView);
	VkFramebufferCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	info.renderPass = rp.rp;
	info.attachmentCount = (uint)att.size();
	info.pAttachments = att.data();
	info.width = size.x;
	info.height = size.y;
	info.layers = layers;
	if (vkCreateFramebuffer(d.d, &info, nullptr, &fb) != VK_SUCCESS) {
		throw std::runtime_error("failed to create framebuffer!");
	}
}
FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept: d(other.d) {
	fb = other.fb;
	size = other.size;
	other.fb = nullptr;
}
FrameBuffer::~FrameBuffer() {
	if (fb != nullptr) vkDestroyFramebuffer(d.d, fb, nullptr);
}
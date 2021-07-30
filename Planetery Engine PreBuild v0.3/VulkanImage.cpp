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

Image::Image(LogicalDevice& d, vec3 texSize, Flags<TextureFeature> texFeature,
  Flags<TextureUseType> texUsage, uint texDimension, VkFormat texFormat,
  uint mipLevels, uint layers, uint subsamples): d(d) {
	size = texSize;
	feature = texFeature;
	usage = texUsage;
	dimension = texDimension;
	// TODO: Add direct mappable image
	VkImageCreateInfo iInfo{};
	iInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	iInfo.flags = (VkImageCreateFlags)texFeature;
	iInfo.imageType = texDimension - 1; //Hmm...
	iInfo.format = texFormat;
	iInfo.extent = VkExtent3D{texSize.x, texSize.y, texSize.z};
	iInfo.mipLevels = mipLevels;
	iInfo.arrayLayers = layers;
	iInfo.samples = subsamples;
	iInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	iInfo.usage = (VkImageUsageFlags)texUsage;
	iInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	//iInfo.queueFamilyIndexCount;
	//iInfo.pQueueFamilyIndices;
	iInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	if (vkCreateImage(d.d, &iInfo, nullptr, &img) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(d.d, img, &memRequirements);
	uint memTypeIndex = d.pd.getMemoryTypeIndex(
	  memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	// TODO: Add fallback
	if (memTypeIndex == -1) throw "VulkanFailedToGetMemoryType";
	// TODO: Fix allocator for custom allocation in PhysicalDevice
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = memTypeIndex;
	if (vkAllocateMemory(d.d, &allocInfo, nullptr, &dm) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}
	vkBindImageMemory(d.d, img, dm, 0);


}
Image::Image(Image&& other) noexcept: d(other.d) {
	img = other.img;
	other.img = nullptr;
}
Image::~Image() {
	if (img != nullptr) vkDestroyImage(d.d, img, nullptr);
	if (dm != nullptr) vkFreeMemory(d.d, dm, nullptr);
}

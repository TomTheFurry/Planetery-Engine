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

CommendPool::CommendPool(LogicalDevice& device, CommendPoolType type):
  d(device) {
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = d.queueIndex;
	switch (type) {
	case CommendPoolType::Default: poolInfo.flags = 0; break;
	case CommendPoolType::Shortlived:
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		break;
	case CommendPoolType::Resetable:
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		break;
	default: throw "VulkanInvalidEnum";
	}
	if (vkCreateCommandPool(d.d, &poolInfo, nullptr, &cp) != VK_SUCCESS) {
		throw "VulkanCreateCommandPoolFailed";
	}
}
CommendPool::CommendPool(CommendPool&& other) noexcept: d(other.d) {
	cp = other.cp;
	other.cp = nullptr;
}
CommendPool::~CommendPool() {
	if (cp != nullptr) vkDestroyCommandPool(d.d, cp, nullptr);
}

CommendBuffer::CommendBuffer(CommendPool& pool): cp(pool) {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = cp.cp;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
	if (vkAllocateCommandBuffers(cp.d.d, &allocInfo, &cb) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffer!");
	}
}
CommendBuffer::CommendBuffer(CommendBuffer&& other) noexcept: cp(other.cp) {
	cb = other.cb;
	other.cb = nullptr;
}
CommendBuffer::~CommendBuffer() {
	if (cb != nullptr) vkFreeCommandBuffers(cp.d.d, cp.cp, 1, &cb);
}
void CommendBuffer::startRecording(Flags<CommendBufferUsage> usage) {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = (uint)usage;
	beginInfo.pInheritanceInfo = nullptr;
	if (vkBeginCommandBuffer(cb, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}
}
void CommendBuffer::cmdBeginRender(
  const RenderPass& rp, const FrameBuffer& fb, vec4 bgColor) {
	VkRenderPassBeginInfo rpCmdInfo{};
	rpCmdInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpCmdInfo.renderPass = rp.rp;
	rpCmdInfo.framebuffer = fb.fb;
	rpCmdInfo.renderArea.offset = {0, 0};
	rpCmdInfo.renderArea.extent = {fb.size.x, fb.size.y};
	rpCmdInfo.clearValueCount = 1;
	VkClearValue v{};
	v.color.float32[0] = bgColor.r;
	v.color.float32[1] = bgColor.g;
	v.color.float32[2] = bgColor.b;
	v.color.float32[3] = bgColor.a;
	rpCmdInfo.pClearValues = &v;
	vkCmdBeginRenderPass(cb, &rpCmdInfo, VK_SUBPASS_CONTENTS_INLINE);
}
void CommendBuffer::cmdBind(const ShaderPipeline& p) {
	vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, p.p);
}
void CommendBuffer::cmdBind(
  const VertexBuffer& vb, uint bindingPoint, size_t offset) {
	vkCmdBindVertexBuffers(cb, bindingPoint, 1, &vb.b, &offset);
}
void CommendBuffer::cmdBind(
  const IndexBuffer& ib, VkIndexType dataType, size_t offset) {
	vkCmdBindIndexBuffer(cb, ib.b, offset, dataType);
}
void CommendBuffer::cmdDraw(
  uint vCount, uint iCount, uint vOffset, uint iOffset) {
	vkCmdDraw(cb, vCount, iCount, vOffset, iOffset);
}
void CommendBuffer::cmdDrawIndexed(uint indCount, uint insCount, uint indOffset,
  uint vertOffset, uint insOffset) {
	vkCmdDrawIndexed(cb, indCount, insCount, indOffset, vertOffset, insOffset);
}
void CommendBuffer::cmdCopyBuffer(const Buffer& src, Buffer& dst, size_t size,
  size_t srcOffset, size_t dstOffset) {
	VkBufferCopy bInfo{};
	bInfo.size = size;
	bInfo.srcOffset = srcOffset;
	bInfo.dstOffset = dstOffset;
	vkCmdCopyBuffer(cb, src.b, dst.b, 1, &bInfo);
}
void CommendBuffer::cmdEndRender() { vkCmdEndRenderPass(cb); }
void CommendBuffer::endRecording() {
	if (vkEndCommandBuffer(cb) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}
#else
module Vulkan;
#endif
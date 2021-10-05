module;
#include "Marco.h"
module Vulkan: CommendImp;
import: Commend;
import: Enum;
import: Buffer;
import: Device;
import: Pipeline;
import: Image;
import: Descriptor;
import: Sync;
import std.core;
import Define;
import Logger;
import "Assert.h";
import "VulkanExtModule.h";
using namespace vk;

CommendPool::CommendPool(
  QueuePool& queuePool, uint queueGroupIndex, Flags<CommendPoolType> type):
  qp(queuePool),
  d(queuePool.getDevice()) {
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueGroupIndex;
	poolInfo.flags = (VkCommandPoolCreateFlags)type;
	if (vkCreateCommandPool(d.d, &poolInfo, nullptr, &cp) != VK_SUCCESS) {
		throw "VulkanCreateCommandPoolFailed";
	}
}
CommendPool::CommendPool(CommendPool&& other) noexcept:
  d(other.d), qp(other.qp) {
	cp = other.cp;
	other.cp = nullptr;
}
CommendPool::~CommendPool() {
	if (cp != nullptr) vkDestroyCommandPool(d.d, cp, nullptr);
}
CommendBuffer CommendPool::makeCommendBuffer() { return CommendBuffer(*this); }

CommendBuffer::CommendBuffer(CommendPool& pool): cp(pool) {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = cp.cp;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
	if (vkAllocateCommandBuffers(cp.getDevice().d, &allocInfo, &cb)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffer!");
	}
}
CommendBuffer::CommendBuffer(CommendBuffer&& other) noexcept: cp(other.cp) {
	cb = other.cb;
	other.cb = nullptr;
}
CommendBuffer::~CommendBuffer() {
	if (cb != nullptr) vkFreeCommandBuffers(cp.getDevice().d, cp.cp, 1, &cb);
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
void CommendBuffer::cmdBind(const RenderPipeline& p) {
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
void CommendBuffer::cmdBind(const DescriptorSet& ds, const RenderPipeline& p) {
	vkCmdBindDescriptorSets(
	  cb, VK_PIPELINE_BIND_POINT_GRAPHICS, p.pl, 0, 1, &ds.ds, 0, nullptr);
}
void CommendBuffer::cmdChangeState(Image& target, TextureSubRegion subRegion,
  ImageRegionState start, Flags<PipelineStage> srcStage,
  Flags<MemoryAccess> srcAccess, ImageRegionState end,
  Flags<PipelineStage> dstStage, Flags<MemoryAccess> dstAccess) {
	VkImageMemoryBarrier imb{};
	imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imb.srcAccessMask = (VkAccessFlags)srcAccess;
	imb.dstAccessMask = (VkAccessFlags)dstAccess;
	imb.oldLayout = (VkImageLayout)start;
	imb.newLayout = (VkImageLayout)end;
	imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.image = target.img;
	imb.subresourceRange = (VkImageSubresourceRange&)subRegion;
	vkCmdPipelineBarrier(cb, (VkPipelineStageFlags)srcStage,
	  (VkPipelineStageFlags)dstStage, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr,
	  0, nullptr, 1, &imb);
}

void CommendBuffer::cmdPushConstants(const RenderPipeline& p,
  Flags<ShaderType> shaders, uint offset, uint size, const void* data) {
	vkCmdPushConstants(
	  cb, p.pl, (VkShaderStageFlags)shaders, offset, size, data);
}

void CommendBuffer::cmdDraw(
  uint vCount, uint iCount, uint vOffset, uint iOffset) {
	vkCmdDraw(cb, vCount, iCount, vOffset, iOffset);
}
void CommendBuffer::cmdDrawIndexed(uint indCount, uint insCount, uint indOffset,
  uint vertOffset, uint insOffset) {
	vkCmdDrawIndexed(cb, indCount, insCount, indOffset, vertOffset, insOffset);
}
void CommendBuffer::cmdCopy(const Buffer& src, Buffer& dst, size_t size,
  size_t srcOffset, size_t dstOffset) {
	VkBufferCopy bInfo{};
	bInfo.size = size;
	bInfo.srcOffset = srcOffset;
	bInfo.dstOffset = dstOffset;
	vkCmdCopyBuffer(cb, src.b, dst.b, 1, &bInfo);
}
void CommendBuffer::cmdCopy(const Buffer& src, Image& dst,
  ImageRegionState usage, TextureSubLayers subLayers, uvec3 inputTextureSize,
  size_t inputDataOffset, uvec3 copyRegion, ivec3 copyOffset) {
	VkBufferImageCopy bic{};
	bic.bufferOffset = inputDataOffset;
	bic.bufferRowLength = inputTextureSize.x;
	bic.bufferImageHeight = inputTextureSize.y;
	bic.imageSubresource = (VkImageSubresourceLayers&)subLayers;
	bic.imageOffset = VkOffset3D{copyOffset.x, copyOffset.y, copyOffset.z};
	bic.imageExtent = VkExtent3D{copyRegion.x, copyRegion.y, copyRegion.z};
	vkCmdCopyBufferToImage(cb, src.b, dst.img, (VkImageLayout)usage, 1, &bic);
}
void CommendBuffer::cmdCopy(const Image& src, ImageRegionState usage,
  Buffer& dst, TextureSubLayers subLayers, uvec3 copyRegion, ivec3 copyOffset,
  uvec3 outputTextureSize, size_t outputDataOffset) {
	VkBufferImageCopy bic{};
	bic.bufferOffset = outputDataOffset;
	bic.bufferRowLength = outputTextureSize.x;
	bic.bufferImageHeight = outputTextureSize.y;
	bic.imageSubresource = (VkImageSubresourceLayers&)subLayers;
	bic.imageOffset = VkOffset3D{copyOffset.x, copyOffset.y, copyOffset.z};
	bic.imageExtent = VkExtent3D{copyRegion.x, copyRegion.y, copyRegion.z};
	vkCmdCopyImageToBuffer(cb, src.img, (VkImageLayout)usage, dst.b, 1, &bic);
}



void CommendBuffer::cmdEndRender() { vkCmdEndRenderPass(cb); }
void CommendBuffer::endRecording() {
	if (vkEndCommandBuffer(cb) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

Fence vk::CommendBuffer::quickSubmit(Queue& queue) {
	Fence fc{cp.getDevice()};
	queue.submit(*this, nullptr, PipelineStage::None, nullptr, &fc);
	return fc;
}

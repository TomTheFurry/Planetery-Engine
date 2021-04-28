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

VertexAttribute::VertexAttribute() {}
void VertexAttribute::addAttribute(
  uint bindingPoint, uint size, VkFormat format) {
	auto& d = attributes.emplace_back();
	d.location = (uint)attributes.size() - 1;
	d.binding = bindingPoint;
	d.format = format;
	d.offset = strideSize;
	strideSize += size;
}
void VertexAttribute::addBindingPoint(uint stride, BufferInputRate rate) {
	auto& b = bindingPoints.emplace_back();
	b.binding = (uint)bindingPoints.size() - 1;
	b.inputRate = (VkVertexInputRate)rate;
	b.stride = stride;
}
VkPipelineVertexInputStateCreateInfo VertexAttribute::getStructForPipeline() {
	for (auto& bp : bindingPoints)
		if (bp.stride == uint(-1)) bp.stride = strideSize;
	VkPipelineVertexInputStateCreateInfo vis{};
	vis.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vis.pVertexAttributeDescriptions = attributes.data();
	vis.pVertexBindingDescriptions = bindingPoints.data();
	vis.vertexAttributeDescriptionCount = (uint)attributes.size();
	vis.vertexBindingDescriptionCount = (uint)bindingPoints.size();
	return vis;
}

RenderPass::RenderPass(LogicalDevice& device): d(device) {}
RenderPass::RenderPass(RenderPass&& o) noexcept: d(o.d) {
	rp = o.rp;
	o.rp = nullptr;
}
void RenderPass::complete() {
	std::vector<VkSubpassDescription> subpassDes;
	std::list<std::vector<VkAttachmentReference>> attRefs;
	subpassDes.reserve(attachmentTypes.size());
	for (uint i = 0; i < subpasses.size(); i++) {
		auto& attRef = attRefs.emplace_back();
		attRef.reserve(subpasses.at(i).size());
		for (uint j = 0; j < subpasses.at(i).size(); j++) {
			attRef.push_back(VkAttachmentReference{
			  .attachment = subpasses.at(i).at(j),
			  .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			});
		}
		subpassDes.push_back(VkSubpassDescription{
		  .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		  .colorAttachmentCount = (uint)attRef.size(),
		  .pColorAttachments = attRef.data(),
		});
	}
	VkRenderPassCreateInfo renderPassInfo{
	  .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
	  .attachmentCount = (uint)attachmentTypes.size(),
	  .pAttachments = attachmentTypes.data(),
	  .subpassCount = (uint)subpassDes.size(),
	  .pSubpasses = subpassDes.data(),
	  .dependencyCount = (uint)subpassDependencies.size(),
	  .pDependencies = subpassDependencies.data(),
	};
	if (vkCreateRenderPass(d.d, &renderPassInfo, nullptr, &rp) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}
RenderPass::~RenderPass() {
	if (rp != nullptr) vkDestroyRenderPass(d.d, rp, nullptr);
}

ShaderPipeline::ShaderPipeline(LogicalDevice& device): d(device) {}
ShaderPipeline::ShaderPipeline(ShaderPipeline&& other) noexcept: d(other.d) {}
ShaderPipeline::~ShaderPipeline() {
	if (pl != nullptr) vkDestroyPipelineLayout(d.d, pl, nullptr);
	if (p != nullptr) vkDestroyPipeline(d.d, p, nullptr);
}
void ShaderPipeline::complete(std::vector<const ShaderCompiled*> shaderModules,
  VertexAttribute& va, VkViewport viewport, const RenderPass& renderPass) {
	std::vector<VkPipelineShaderStageCreateInfo> sInfos;
	sInfos.reserve(shaderModules.size());
	for (auto& sm : shaderModules) sInfos.emplace_back(sm->getCreateInfo());

	VkPipelineVertexInputStateCreateInfo vertInputBinding =
	  va.getStructForPipeline();

	VkPipelineInputAssemblyStateCreateInfo vertInputType{};
	vertInputType.sType =
	  VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	vertInputType.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	vertInputType.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState{};
	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = VkExtent2D{.width = (uint)viewport.width,
	  .height = (uint)viewport.height};	 // HACK HERE
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType =
	  VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;	// Optional
	rasterizer.depthBiasClamp = 0.0f;			// Optional
	rasterizer.depthBiasSlopeFactor = 0.0f;		// Optional

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType =
	  VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;			 // Optional
	multisampling.pSampleMask = nullptr;			 // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE;	 // Optional
	multisampling.alphaToOneEnable = VK_FALSE;		 // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask =
	  VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
	  | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;	 // Optional
	colorBlendAttachment.dstColorBlendFactor =
	  VK_BLEND_FACTOR_ZERO;											 // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;			 // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;	 // Optional
	colorBlendAttachment.dstAlphaBlendFactor =
	  VK_BLEND_FACTOR_ZERO;								  // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;  // Optional
	colorBlending.sType =
	  VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;  // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;	 // Optional
	colorBlending.blendConstants[1] = 0.0f;	 // Optional
	colorBlending.blendConstants[2] = 0.0f;	 // Optional
	colorBlending.blendConstants[3] = 0.0f;	 // Optional

	// VkPipelineDynamicStateCreateInfo dynamicState;
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;			   // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr;		   // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0;	   // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr;  // Optional
	if (vkCreatePipelineLayout(d.d, &pipelineLayoutInfo, nullptr, &pl)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = (uint)sInfos.size();
	pipelineInfo.pStages = sInfos.data();
	pipelineInfo.pVertexInputState = &vertInputBinding;
	pipelineInfo.pInputAssemblyState = &vertInputType;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;	// Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;  // Optional
	pipelineInfo.layout = pl;
	pipelineInfo.renderPass =
	  renderPass.rp;  // TODO: add ref count in renderPass Ob
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
	pipelineInfo.basePipelineIndex = -1;			   // Optional
	if (vkCreateGraphicsPipelines(
		  d.d, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &p)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}
#else
module Vulkan;
#endif
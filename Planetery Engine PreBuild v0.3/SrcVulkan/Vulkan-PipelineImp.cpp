module Vulkan: PipelineImp;
import: Pipeline;
import: Enum;
import: Device;
import: Descriptor;
import: Shader;
import std.core;
import Define;
import Logger;
import "Assert.h";
import "VulkanExtModule.h";
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




RenderPass::Attachment::Attachment(VkFormat colorFormat,
  TextureActiveUseType currentActiveUsage, AttachmentReadOp readOp,
  TextureActiveUseType outputActiveUsage, AttachmentWriteOp writeOp, uint sam) {
	flags = 0;
	format = colorFormat;
	samples = (VkSampleCountFlagBits)sam;
	loadOp = (VkAttachmentLoadOp)readOp;
	storeOp = (VkAttachmentStoreOp)writeOp;
	stencilLoadOp = (VkAttachmentLoadOp)AttachmentReadOp::Undefined;
	stencilStoreOp = (VkAttachmentStoreOp)AttachmentWriteOp::Undefined;
	initialLayout = (VkImageLayout)currentActiveUsage;
	finalLayout = (VkImageLayout)outputActiveUsage;
}
RenderPass::Attachment::Attachment(VkFormat depthStencilformat,
  TextureActiveUseType currentActiveUsage, AttachmentReadOp depthReadOp,
  AttachmentReadOp sReadOp, TextureActiveUseType outputActiveUsage,
  AttachmentWriteOp depthWriteOp, AttachmentWriteOp sWriteOp, uint sam) {
	flags = 0;
	format = depthStencilformat;
	samples = (VkSampleCountFlagBits)sam;
	loadOp = (VkAttachmentLoadOp)depthReadOp;
	storeOp = (VkAttachmentStoreOp)depthWriteOp;
	stencilLoadOp = (VkAttachmentLoadOp)sReadOp;
	stencilStoreOp = (VkAttachmentStoreOp)sWriteOp;
	initialLayout = (VkImageLayout)currentActiveUsage;
	finalLayout = (VkImageLayout)outputActiveUsage;
}

enum class _AtmUsage {
	InputColor = 1,
	InputDepthStencil = 2,
	Color = 4,
	DepthStencil = 8,
	Resolve = 16,
	Forward = 32,
};
TextureActiveUseType _getSubPassAtmGoodActiveUseType(Flags<_AtmUsage> v) {
	switch (v.toVal()) {
	case Flags(_AtmUsage::InputColor).toVal():
		return TextureActiveUseType::ReadOnlyShader;
	case Flags(_AtmUsage::InputDepthStencil).toVal():
		return TextureActiveUseType::ReadOnlyAttachmentDepthStencil;

	case Flags(_AtmUsage::Color).toVal():
		return TextureActiveUseType::AttachmentColor;
	case Flags(_AtmUsage::DepthStencil).toVal():
		return TextureActiveUseType::AttachmentDepthStencil;

	case Flags(_AtmUsage::Forward).toVal():
		return TextureActiveUseType::Undefined;	 // dont care

	default:
		if (v.has(_AtmUsage::Forward))
			throw "VulkanSubPassInvalidAttachmentUsage:UniqueForward";
		if (v.has(_AtmUsage::Color) && v.has(_AtmUsage::DepthStencil))
			throw "VulkanSubPassInvalidAttachmentUsage:ColorOrDepthStencil";
		return TextureActiveUseType::General;
	}
}

RenderPass::SubPass::SubPass(std::initializer_list<uint> inputColorAtm,
  std::initializer_list<uint> inputDepthStencilAtm,
  std::initializer_list<uint> colorAtm,
  std::initializer_list<uint> depthStencilAtm,
  std::initializer_list<uint> resolveAtm,
  std::initializer_list<uint> forwardAtm):
  preserve(forwardAtm) {
	if (!std::empty(depthStencilAtm)
		&& depthStencilAtm.size() != colorAtm.size())
		throw "VulkanSubpassInvalidDepthStencilAttachmentCount";
	if (!std::empty(resolveAtm) && resolveAtm.size() != colorAtm.size())
		throw "VulkanSubpassInvalidResolveAttachmentCount";
	std::map<uint, Flags<_AtmUsage>> atm{};
	for (auto& id : inputColorAtm) atm[id] |= _AtmUsage::InputColor;
	for (auto& id : inputDepthStencilAtm)
		atm[id] |= _AtmUsage::InputDepthStencil;
	for (auto& id : colorAtm) atm[id] |= _AtmUsage::Color;
	for (auto& id : depthStencilAtm) atm[id] |= _AtmUsage::DepthStencil;
	for (auto& id : resolveAtm) atm[id] |= _AtmUsage::Resolve;
	atm.erase(uint(-1));
	std::unordered_map<uint, TextureActiveUseType> atmType{};
	atmType.reserve(atm.size());
	for (auto it : atm) {
		atmType[it.first] = _getSubPassAtmGoodActiveUseType(it.second);
	}
	// Push to vector
	input.reserve(inputColorAtm.size() + inputDepthStencilAtm.size());
	for (auto& id : inputColorAtm)
		input.push_back(
		  VkAttachmentReference{id, (VkImageLayout)atmType.at(id)});
	for (auto& id : inputDepthStencilAtm)
		input.push_back(
		  VkAttachmentReference{id, (VkImageLayout)atmType.at(id)});

	color.reserve(colorAtm.size());
	for (auto& id : colorAtm)
		color.push_back(
		  VkAttachmentReference{id, (VkImageLayout)atmType.at(id)});

	depthStencil.reserve(depthStencilAtm.size());
	for (auto& id : depthStencilAtm)
		depthStencil.push_back(
		  VkAttachmentReference{id, (VkImageLayout)atmType.at(id)});

	resolve.reserve(resolveAtm.size());
	for (auto& id : resolveAtm)
		resolve.push_back(
		  VkAttachmentReference{id, (VkImageLayout)atmType.at(id)});
}

RenderPass::SubPass::operator VkSubpassDescription() const {
	VkSubpassDescription spInfo{};
	spInfo.flags = 0;
	spInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	spInfo.inputAttachmentCount = input.size();
	spInfo.pInputAttachments = input.empty() ? nullptr : input.data();
	spInfo.colorAttachmentCount = color.size();
	spInfo.pColorAttachments = color.empty() ? nullptr : color.data();
	spInfo.pResolveAttachments = resolve.empty() ? nullptr : resolve.data();
	spInfo.pDepthStencilAttachment =
	  depthStencil.empty() ? nullptr : depthStencil.data();
	spInfo.preserveAttachmentCount = preserve.size();
	spInfo.pPreserveAttachments = preserve.empty() ? nullptr : preserve.data();
	return spInfo;
}

RenderPass::SubPassDependency::SubPassDependency(uint spId,
  PipelineStage srcStage, MemoryAccess srcAccess, PipelineStage dstStage,
  MemoryAccess dstAccess, bool perPixelIndependent) {
	if (spId == uint(-1)) spId = VK_SUBPASS_EXTERNAL;
	srcSubpass = spId;
	dstSubpass = spId;
	srcStageMask = (VkPipelineStageFlags)srcStage;
	dstStageMask = (VkPipelineStageFlags)dstStage;
	srcAccessMask = (VkAccessFlags)srcAccess;
	dstAccessMask = (VkAccessFlags)dstAccess;
	dependencyFlags = perPixelIndependent
					  ? VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT
					  : 0;
}
RenderPass::SubPassDependency::SubPassDependency(uint srcSpId,
  PipelineStage srcStage, MemoryAccess srcAccess, uint dstSpId,
  PipelineStage dstStage, MemoryAccess dstAccess, bool perPixelIndependent) {
	if (srcSpId == uint(-1)) srcSpId = VK_SUBPASS_EXTERNAL;
	if (dstSpId == uint(-1)) dstSpId = VK_SUBPASS_EXTERNAL;
	srcSubpass = srcSpId;
	dstSubpass = dstSpId;
	srcStageMask = (VkPipelineStageFlags)srcStage;
	dstStageMask = (VkPipelineStageFlags)dstStage;
	srcAccessMask = (VkAccessFlags)srcAccess;
	dstAccessMask = (VkAccessFlags)dstAccess;
	dependencyFlags = perPixelIndependent
					  ? VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT
					  : 0;
}

RenderPass::RenderPass(LogicalDevice& device,
  std::initializer_list<Attachment> attachments,
  std::initializer_list<SubPass> subPasses,
  std::initializer_list<SubPassDependency> dependencies):
  d(device) {
	std::vector<VkSubpassDescription> spDes{};
	spDes.reserve(subPasses.size());
	for (auto& sp : subPasses) spDes.push_back((VkSubpassDescription)sp);
	VkRenderPassCreateInfo renderPassInfo{
	  .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
	  .attachmentCount = (uint)attachments.size(),
	  .pAttachments = std::data(attachments),
	  .subpassCount = (uint)subPasses.size(),
	  .pSubpasses = std::data(spDes),
	  .dependencyCount = (uint)dependencies.size(),
	  .pDependencies = std::data(dependencies),
	};
	if (vkCreateRenderPass(d.d, &renderPassInfo, nullptr, &rp) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass!");
	}
}
RenderPass::RenderPass(RenderPass&& o) noexcept: d(o.d) {
	rp = o.rp;
	o.rp = nullptr;
}
RenderPass::~RenderPass() {
	if (rp != nullptr) vkDestroyRenderPass(d.d, rp, nullptr);
}

RenderPipeline::PushConstantLayout::PushConstantLayout(
  uint o, uint s, Flags<ShaderType> shaders) {
	stageFlags = (VkShaderStageFlags)shaders;
	offset = o;
	size = s;
}

RenderPipeline::ShaderStage::ShaderStage(
  ShaderCompiled& shader, const char* entryName) {
	sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pNext = nullptr;
	flags = 0;
	stage = (VkShaderStageFlagBits)shader.shaderType;
	module = shader.sm;
	pName = entryName;
	pSpecializationInfo = nullptr;
}

RenderPipeline::RenderPipeline(RenderPipeline&& other) noexcept: d(other.d) {}
RenderPipeline::~RenderPipeline() {
	if (pl != nullptr) vkDestroyPipelineLayout(d.d, pl, nullptr);
	if (p != nullptr) vkDestroyPipeline(d.d, p, nullptr);
}

// TODO: add support for push constants
RenderPipeline::RenderPipeline(LogicalDevice& d,
  std::initializer_list<DescriptorLayout*> descriptorLayouts,
  std::initializer_list<PushConstantLayout> pushConstants,
  RenderPass& renderPass, uint32_t subpassId,
  std::initializer_list<ShaderStage> stages, VertexAttribute& vertAttribute,
  PrimitiveTopology vertTopology, bool primitiveRestartByIndex,
  std::initializer_list<VkViewport> viewports,
  std::initializer_list<VkRect2D> scissors, bool rasterizerClampDepth,
  bool rasterizerDiscard, PolygonMode polygonMode, Flags<CullMode> cullMode,
  FrontDirection frontDirection, std::optional<DepthBias> depthBias,
  float lineWidth, uint sampleCount, bool rasterizerSetAlphaToOne,
  std::optional<DepthStencilSettings> depthStencilSettings,
  LogicOperator colorBlendLogicOperator,
  std::initializer_list<AttachmentBlending> attachmentBlending,
  vec4 blendConstants = vec4(1.f)):
  d(d) {
	{
		VkPipelineLayoutCreateInfo layoutInfo{};
		std::vector<VkDescriptorSetLayout> vkDescLayout{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = std::size(descriptorLayouts);
		vkDescLayout.reserve(std::size(descriptorLayouts));
		for (auto ptr : descriptorLayouts) vkDescLayout.push_back(ptr->dsl);
		layoutInfo.pSetLayouts = vkDescLayout.data();
		// TODO: add support for push constants
		layoutInfo.pushConstantRangeCount = std::size(pushConstants);
		layoutInfo.pPushConstantRanges =
		  (VkPushConstantRange*)std::data(pushConstants);
		if (vkCreatePipelineLayout(d.d, &layoutInfo, nullptr, &pl)
			!= VK_SUCCESS) {
			throw std::runtime_error(
			  "failed to create graphics pipeline layout!");
		}
	}
	//TODO: add support for reuse
	VkPipeline basePipeline = nullptr;
	bool reuseable = false;

	VkGraphicsPipelineCreateInfo cInfo{};
	cInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	// Reuse settings
	{
		if (basePipeline != nullptr) {
			cInfo.flags |= VK_PIPELINE_CREATE_DERIVATIVE_BIT;
		}
		cInfo.basePipelineHandle = basePipeline;
		cInfo.basePipelineIndex = -1;
		if (reuseable) cInfo.flags |= VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
	}

	// Stages Settings
	{
		cInfo.stageCount = std::size(stages);
		cInfo.pStages = (VkPipelineShaderStageCreateInfo*)std::data(stages);
	}

	// Vertex Input Attrubutes
	VkPipelineVertexInputStateCreateInfo vertInputInfo{};
	{
		// TODO: Add dynamic
		vertInputInfo = vertAttribute.getStructForPipeline();
		cInfo.pVertexInputState = &vertInputInfo;
	}

	// Vertex Input Topology
	VkPipelineInputAssemblyStateCreateInfo vertTopologyInfo{};
	{
		// TODO: Inore if mesh shader enabled
		vertTopologyInfo.sType =
		  VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		vertTopologyInfo.topology = (VkPrimitiveTopology)vertTopology;
		vertTopologyInfo.primitiveRestartEnable = primitiveRestartByIndex;
		cInfo.pInputAssemblyState = &vertTopologyInfo;
	}

	// TODO: Tessellation Settings
	cInfo.pTessellationState = nullptr;

	// Viewports & Scissors
	VkPipelineViewportStateCreateInfo viewportScissorInfo{};
	{
		// TODO: Inore if rasterization disabled
		viewportScissorInfo.sType =
		  VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportScissorInfo.viewportCount = std::size(viewports);
		viewportScissorInfo.pViewports = std::data(viewports);
		viewportScissorInfo.scissorCount = std::size(scissors);
		viewportScissorInfo.pScissors = std::data(scissors);
		cInfo.pViewportState = &viewportScissorInfo;
	}

	// Rasterization Settings
	VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
	{
		rasterizationInfo.sType =
		  VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationInfo.depthClampEnable = rasterizerClampDepth;
		rasterizationInfo.rasterizerDiscardEnable = rasterizerDiscard;
		rasterizationInfo.polygonMode = (VkPolygonMode)polygonMode;
		rasterizationInfo.cullMode = cullMode;
		rasterizationInfo.frontFace = (VkFrontFace)frontDirection;
		rasterizationInfo.depthBiasEnable = depthBias.has_value();
		if (depthBias) {
			rasterizationInfo.depthBiasConstantFactor =
			  depthBias->constantFactor;
			rasterizationInfo.depthBiasClamp = depthBias->clamp;
			rasterizationInfo.depthBiasSlopeFactor = depthBias->slopeFactor;
		}
		rasterizationInfo.lineWidth = lineWidth;
		cInfo.pRasterizationState = &rasterizationInfo;
	}

	// Sampling Settings
	VkPipelineMultisampleStateCreateInfo samplingInfo{};
	{
		// TODO: Inore if rasterization disabled
		samplingInfo.sType =
		  VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		samplingInfo.rasterizationSamples = (VkSampleCountFlagBits)sampleCount;
		// TODO: Add support for sampleShading
		samplingInfo.sampleShadingEnable = false;
		// samplingInfo.minSampleShading;
		// TODO: Add support for sampleMask
		// samplingInfo.pSampleMask;
		// TODO: Add support for alphaToCoverageEnable
		samplingInfo.alphaToCoverageEnable = false;
		// TODO: check for feature: alpha to one
		samplingInfo.alphaToOneEnable = rasterizerSetAlphaToOne;
		cInfo.pMultisampleState = &samplingInfo;
	}

	// DepthStencil Settings - only if renderPass has depthStencil
	// TODO: Check if renderPass has depthStencil
	cInfo.pDepthStencilState =
	  depthStencilSettings.has_value()
		? (VkPipelineDepthStencilStateCreateInfo*)&depthStencilSettings.value()
		: nullptr;

	// Color Blending Settings - only if renderPass has color
	VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
	{
		// TODO: Inore if rasterization disabled
		// TODO: Inore if no color attachments
		colorBlendInfo.sType =
		  VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		// TODO: Check feature: logic Op
		colorBlendInfo.logicOpEnable =
		  colorBlendLogicOperator != LogicOperator::None;
		colorBlendInfo.logicOp = (VkLogicOp)colorBlendLogicOperator;
		// TODO: Check and handle feature: independent blending
		// TODO: Check attachment count > renderPass max color attachment index
		colorBlendInfo.attachmentCount = std::size(attachmentBlending);
		colorBlendInfo.pAttachments = std::data(attachmentBlending);
		colorBlendInfo.blendConstants[0] = blendConstants.r;
		colorBlendInfo.blendConstants[1] = blendConstants.g;
		colorBlendInfo.blendConstants[2] = blendConstants.b;
		colorBlendInfo.blendConstants[3] = blendConstants.a;
		cInfo.pColorBlendState = &colorBlendInfo;
	}

	// TODO: Dynamic Settings
	cInfo.pDynamicState = nullptr;
	cInfo.layout = pl;
	cInfo.renderPass = renderPass.rp;
	cInfo.subpass = subpassId;

	if (vkCreateGraphicsPipelines(d.d, VK_NULL_HANDLE, 1, &cInfo, nullptr, &p)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}
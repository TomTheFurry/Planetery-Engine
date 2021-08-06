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

ShaderPipeline::ShaderPipeline(LogicalDevice& device): d(device) {}
ShaderPipeline::ShaderPipeline(ShaderPipeline&& other) noexcept: d(other.d) {}
ShaderPipeline::~ShaderPipeline() {
	if (pl != nullptr) vkDestroyPipelineLayout(d.d, pl, nullptr);
	if (p != nullptr) vkDestroyPipeline(d.d, p, nullptr);
}
void vk::ShaderPipeline::bind(const DescriptorLayout& dsl) {
	_dsl.emplace_back(dsl.dsl);
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
	rasterizer.depthBiasConstantFactor = 0.0f;	// TODO
	rasterizer.depthBiasClamp = 0.0f;			// TODO
	rasterizer.depthBiasSlopeFactor = 0.0f;		// TODO

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType =
	  VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;			 // TODO
	multisampling.pSampleMask = nullptr;			 // TODO
	multisampling.alphaToCoverageEnable = VK_FALSE;	 // TODO
	multisampling.alphaToOneEnable = VK_FALSE;		 // TODO

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask =
	  VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
	  | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;	  // TODO
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // TODO
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;			  // TODO
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;	  // TODO
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // TODO
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;			  // TODO
	colorBlending.sType =
	  VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;  // TODO
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;	 // TODO
	colorBlending.blendConstants[1] = 0.0f;	 // TODO
	colorBlending.blendConstants[2] = 0.0f;	 // TODO
	colorBlending.blendConstants[3] = 0.0f;	 // TODO

	// VkPipelineDynamicStateCreateInfo dynamicState;
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = _dsl.size();
	pipelineLayoutInfo.pSetLayouts = _dsl.data();	   // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0;	   // TODO
	pipelineLayoutInfo.pPushConstantRanges = nullptr;  // TODO
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
	pipelineInfo.pDepthStencilState = nullptr;	// TODO
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;  // TODO
	pipelineInfo.layout = pl;
	pipelineInfo.renderPass =
	  renderPass.rp;  // TODO: add ref count in renderPass Ob
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // TODO
	pipelineInfo.basePipelineIndex = -1;			   // TODO
	if (vkCreateGraphicsPipelines(
		  d.d, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &p)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}
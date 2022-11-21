#include "precompiled/pch.hpp"

#include "pipeline.hpp"
#include "utilities/clock.hpp"

namespace RDE
{
namespace Vulkan
{

void Pipeline::create(VkDevice device, VkAllocationCallbacks* allocator, const Swapchain& swapchain, VkSampleCountFlagBits msaaSamples,
                      VkDescriptorSetLayout uboDescriptorSetLayout, VkDescriptorSetLayout samplerDescriptorSetLayout, VkRenderPass renderPass)
{
    RDE_PROFILE_SCOPE

    // Shader stage
    auto vertexShaderCode = FileParser::read("assets/shaders/vert.spv");
    auto fragmentShaderCode = FileParser::read("assets/shaders/frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(device, allocator, vertexShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(device, allocator, fragmentShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main"; // Entry point

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main"; // Entry point

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {vertShaderStageInfo, fragShaderStageInfo};

    // Descriptions
    const BindingDescriptions bindingDescriptions{};
    const AttributeDescriptions attributeDescriptions{};

    // Vertex input state
    std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions = {bindingDescriptions.getVertexBindingDescription(),
                                                                                   bindingDescriptions.getInstanceBindingDescription()};

    std::array<VkVertexInputAttributeDescription, 3> vertexAttrDesc = attributeDescriptions.getVertexAttributeDescriptions();
    std::array<VkVertexInputAttributeDescription, 4> instanceAttrDesc = attributeDescriptions.getInstanceAttributeDescriptions();

    // pos, index, uv
    // transformation matrix columns 0, 1, 2, 3
    std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions =         // NOLINT
        {vertexAttrDesc[0],   vertexAttrDesc[1],   vertexAttrDesc[2],                         // NOLINT
         instanceAttrDesc[0], instanceAttrDesc[1], instanceAttrDesc[2], instanceAttrDesc[3]}; // NOLINT

    VkPipelineVertexInputStateCreateInfo inputInfo{};
    inputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    // Bindings descriptions are binding spacings between data, per-vertex or
    // per-instance
    inputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindingDescriptions.size());
    inputInfo.pVertexBindingDescriptions = vertexInputBindingDescriptions.data();

    // Attributes are passed to vertex shader at a certain binding
    inputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributeDescriptions.size());
    inputInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();

    // Input assembly state
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    // Viewport state
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchain.extent.width);
    viewport.height = static_cast<float>(swapchain.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchain.extent;

    VkPipelineViewportStateCreateInfo viewportStateInfo{};
    viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateInfo.viewportCount = 1;
    viewportStateInfo.pViewports = &viewport;
    viewportStateInfo.scissorCount = 1;
    viewportStateInfo.pScissors = &scissor;

    // Rasterization state
    VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
    rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerInfo.depthClampEnable = VK_FALSE;        // Clamp fragments within depth range
    rasterizerInfo.rasterizerDiscardEnable = VK_FALSE; // Discard geometry pass through rasterizer
    rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL; // Fill polygon area with fragments
    rasterizerInfo.lineWidth = 1.0f;
    rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT; // Back-face culling
    rasterizerInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizerInfo.depthBiasEnable = VK_FALSE;
    rasterizerInfo.depthBiasConstantFactor = 0.0f;
    rasterizerInfo.depthBiasClamp = 0.0f;
    rasterizerInfo.depthBiasSlopeFactor = 0.0f;

    // Multisampling state
    VkPipelineMultisampleStateCreateInfo multisampleInfo{};
    multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo.sampleShadingEnable = VK_FALSE;
    multisampleInfo.rasterizationSamples = msaaSamples;
    multisampleInfo.minSampleShading = 1.0f;
    multisampleInfo.pSampleMask = nullptr;
    multisampleInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleInfo.alphaToOneEnable = VK_FALSE;

    // Depth and stencil testing state
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
    depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.depthTestEnable = VK_TRUE;
    depthStencilInfo.depthWriteEnable = VK_TRUE;
    depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilInfo.stencilTestEnable = VK_FALSE;

    // Color blending state
    // Per-framebuffer color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachmentInfo{};
    colorBlendAttachmentInfo.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    // Alpha blending
    colorBlendAttachmentInfo.blendEnable = VK_TRUE;
    colorBlendAttachmentInfo.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachmentInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentInfo.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachmentInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentInfo.alphaBlendOp = VK_BLEND_OP_ADD;

    // Global color blending
    VkPipelineColorBlendStateCreateInfo colorBlendingInfo{};
    colorBlendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendingInfo.logicOpEnable = VK_FALSE; // Setting this to true will void all attachments and use
                                                // bitwise operations
    colorBlendingInfo.attachmentCount = 1;
    colorBlendingInfo.pAttachments = &colorBlendAttachmentInfo;

    // Dynamic state
    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH};

    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateInfo.pDynamicStates = dynamicStates.data();

    // Push Constants
    // VkPushConstantRange pushConstantRange;
    // pushConstantRange.offset = 0;
    // pushConstantRange.size = sizeof(PushConstantObject);
    // pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    // Pipeline layout
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{uboDescriptorSetLayout, samplerDescriptorSetLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data(); // UBO and texture sampler descriptor set layouts
    // pipelineLayoutInfo.pushConstantRangeCount = 1;
    // pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    auto result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, allocator, &m_pipelineLayout);
    RDE_ASSERT_0(result == VK_SUCCESS, "Failed to create pipeline layout!");

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    // Shader stages
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    // Fixed-function states
    pipelineInfo.pVertexInputState = &inputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pViewportState = &viewportStateInfo;
    pipelineInfo.pRasterizationState = &rasterizerInfo;
    pipelineInfo.pMultisampleState = &multisampleInfo;
    pipelineInfo.pDepthStencilState = &depthStencilInfo;
    pipelineInfo.pColorBlendState = &colorBlendingInfo;
    pipelineInfo.pDynamicState = nullptr;
    // Pipeline layout
    pipelineInfo.layout = m_pipelineLayout;
    // Render pass
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0; // Index
    // Pipeline derivatives
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, allocator, &m_graphicsPipeline);
    RDE_ASSERT_0(result == VK_SUCCESS, "Failed to create graphics pipeline!");

    vkDestroyShaderModule(device, vertShaderModule, allocator);
    vkDestroyShaderModule(device, fragShaderModule, allocator);
}

void Pipeline::destroy(VkDevice device, VkAllocationCallbacks* allocator)
{
    vkDestroyPipeline(device, m_graphicsPipeline, allocator);
    vkDestroyPipelineLayout(device, m_pipelineLayout, allocator);
}

void Pipeline::bind(VkCommandBuffer commandBuffer) { vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline); }

[[nodiscard]] VkShaderModule Pipeline::createShaderModule(VkDevice device, VkAllocationCallbacks* allocator, FileParser::FileBufferType shaderCode) const
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderCode.size();
    // Each 4 chars of code is stored as 1 uint32_t
    createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

    VkShaderModule shaderModule;
    auto result = vkCreateShaderModule(device, &createInfo, allocator, &shaderModule);
    RDE_ASSERT_0(result == VK_SUCCESS, "Failed to create shader module!");

    return shaderModule;
}

} // namespace Vulkan
} // namespace RDE
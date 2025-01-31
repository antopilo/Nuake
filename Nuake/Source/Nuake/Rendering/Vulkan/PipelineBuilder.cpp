#include "PipelineBuilder.h"

#include "Nuake/Core/Logger.h"

#include "Nuake/Rendering/Vulkan/VulkanInit.h"

using namespace Nuake;

void PipelineBuilder::Clear()
{
    InputAssembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };

    Rasterizer = { 
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE
    };

    ColorBlendAttachment = {};

    Multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };

    PipelineLayout = {};

    DepthStencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };

    RenderInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };

    ShaderStages.clear();
}

VkPipeline PipelineBuilder::BuildPipeline(VkDevice device)
{
    // make viewport state from our stored viewport and scissor.
    // at the moment we wont support multiple viewports or scissors
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;

    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // setup dummy color blending. We arent using transparent objects yet
    // the blending is just "no blend", but we do write to the color attachment
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.pNext = nullptr;

    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = ColorBlendAttachment.size();
    colorBlending.pAttachments = ColorBlendAttachment.data();

    if (ColorBlendAttachment.size() == 0)
    {
        Logger::Log("Forgot blend attachment?", "vulkan", CRITICAL);
        //assert(false && "Error");
    }

    // completely clear VertexInputStateCreateInfo, as we have no need for it
    VkPipelineVertexInputStateCreateInfo _vertexInputInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

    // build the actual pipeline
    // we now use all of the info structs we have been writing into into this one
    // to create the pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo = { .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    // connect the renderInfo to the pNext extension mechanism
    pipelineInfo.pNext = &RenderInfo;

    pipelineInfo.stageCount = (uint32_t)ShaderStages.size();
    pipelineInfo.pStages = ShaderStages.data();
    pipelineInfo.pVertexInputState = &_vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &InputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &Rasterizer;
    pipelineInfo.pMultisampleState = &Multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &DepthStencil;
    pipelineInfo.layout = PipelineLayout;

    VkDynamicState state[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamicInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    dynamicInfo.pDynamicStates = &state[0];
    dynamicInfo.dynamicStateCount = 2;

    pipelineInfo.pDynamicState = &dynamicInfo;

    // its easy to error out on create graphics pipeline, so we handle it a bit
    // better than the common VK_CHECK case
    VkPipeline newPipeline;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS) 
    {
        Logger::Log("Failed to create graphics pipeline", "vulkan", CRITICAL);
        return VK_NULL_HANDLE; // failed to create graphics pipeline
    }

    return newPipeline;
}

void PipelineBuilder::SetShaders(VkShaderModule vert, VkShaderModule frag)
{
    ShaderStages.clear();

    ShaderStages.push_back(VulkanInit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vert));

    ShaderStages.push_back(VulkanInit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, frag));
}

void PipelineBuilder::SetInputTopology(VkPrimitiveTopology topology)
{
    InputAssembly.topology = topology;
    // we are not going to use primitive restart on the entire tutorial so leave
    // it on false
    InputAssembly.primitiveRestartEnable = VK_FALSE;
}

void PipelineBuilder::SetPolygonMode(VkPolygonMode polygonMode)
{
    Rasterizer.polygonMode = polygonMode;
    Rasterizer.lineWidth = 1.f;
}

void PipelineBuilder::SetCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace)
{
    Rasterizer.cullMode = cullMode;
    Rasterizer.frontFace = frontFace;
}

void PipelineBuilder::SetMultiSamplingNone()
{
    Multisampling.sampleShadingEnable = VK_FALSE;
    // multisampling defaulted to no multisampling (1 sample per pixel)
    Multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    Multisampling.minSampleShading = 1.0f;
    Multisampling.pSampleMask = nullptr;
    // no alpha to coverage either
    Multisampling.alphaToCoverageEnable = VK_FALSE;
    Multisampling.alphaToOneEnable = VK_FALSE;
}

void PipelineBuilder::DisableBlending()
{
    // default write mask
    VkPipelineColorBlendAttachmentState colorBlend = {};
    colorBlend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    // no blending
    colorBlend.blendEnable = VK_FALSE;
	ColorBlendAttachment.push_back(colorBlend);
}

void PipelineBuilder::SetColorAttachment(VkFormat format)
{
    ColorAttachmentformats = { format };
    // connect the format to the renderInfo  structure
    RenderInfo.colorAttachmentCount = 1;
    RenderInfo.pColorAttachmentFormats = ColorAttachmentformats.data();
}

void PipelineBuilder::SetColorAttachments(std::vector<VkFormat>& formats)
{
    ColorAttachmentformats = formats;
    // connect the format to the renderInfo  structure
    RenderInfo.colorAttachmentCount = std::size(formats);
    RenderInfo.pColorAttachmentFormats = ColorAttachmentformats.data();
}

void PipelineBuilder::SetDepthFormat(VkFormat depthFormat)
{
    RenderInfo.depthAttachmentFormat = depthFormat;
}

void PipelineBuilder::DisableDepthTest()
{
    DepthStencil.depthTestEnable = VK_FALSE;
    DepthStencil.depthWriteEnable = VK_FALSE;
    DepthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
    DepthStencil.depthBoundsTestEnable = VK_FALSE;
    DepthStencil.stencilTestEnable = VK_FALSE;
    DepthStencil.front = {};
    DepthStencil.back = {};
    DepthStencil.minDepthBounds = 0.f;
    DepthStencil.maxDepthBounds = 1.f;
}

void PipelineBuilder::EnableDepthTest(bool depthWriteEnable, VkCompareOp op)
{
    DepthStencil.depthTestEnable = VK_TRUE;
    DepthStencil.depthWriteEnable = depthWriteEnable;
    DepthStencil.depthCompareOp = op;
    DepthStencil.depthBoundsTestEnable = VK_FALSE;
    DepthStencil.stencilTestEnable = VK_FALSE;
    DepthStencil.front = {};
    DepthStencil.back = {};
    DepthStencil.minDepthBounds = 1.f;
    DepthStencil.maxDepthBounds = 0.f;
}

void PipelineBuilder::EnableMultiAlphaBlend(size_t count)
{
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState[2] = {};
}

void PipelineBuilder::EnableBlendingAdditive()
{
    VkPipelineColorBlendAttachmentState colorBlend = {};
    colorBlend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlend.blendEnable = VK_TRUE;
    colorBlend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlend.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlend.alphaBlendOp = VK_BLEND_OP_ADD;

	ColorBlendAttachment.push_back(colorBlend);
}

void PipelineBuilder::EnableBlendingAlphaBlend(size_t count)
{
    for (int i = 0; i < count; i++)
    {
        VkPipelineColorBlendAttachmentState colorBlend = {};
        colorBlend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlend.blendEnable = VK_TRUE;
        colorBlend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlend.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlend.alphaBlendOp = VK_BLEND_OP_ADD;

        ColorBlendAttachment.push_back(colorBlend);
    }
}
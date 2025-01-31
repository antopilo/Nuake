#pragma once
#include <volk/volk.h>

#include <vector>

namespace Nuake
{
	class PipelineBuilder
	{
    public:
        std::vector<VkPipelineShaderStageCreateInfo> ShaderStages;

        VkPipelineInputAssemblyStateCreateInfo InputAssembly;
        VkPipelineRasterizationStateCreateInfo Rasterizer;
        std::vector<VkPipelineColorBlendAttachmentState> ColorBlendAttachment = std::vector<VkPipelineColorBlendAttachmentState>();
        VkPipelineMultisampleStateCreateInfo Multisampling;
        VkPipelineLayout PipelineLayout;
        VkPipelineDepthStencilStateCreateInfo DepthStencil;
        VkPipelineRenderingCreateInfo RenderInfo;
        std::vector<VkFormat> ColorAttachmentformats = std::vector<VkFormat>();

	
        PipelineBuilder() { Clear(); }
		~PipelineBuilder() = default;

        void Clear();

        VkPipeline BuildPipeline(VkDevice device);
        void SetShaders(VkShaderModule vert, VkShaderModule frag);

        void SetInputTopology(VkPrimitiveTopology topology);
        void SetPolygonMode(VkPolygonMode polygonMode);
        void SetCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace);
        void SetMultiSamplingNone();
        void DisableBlending();
        void SetColorAttachment(VkFormat format);
        void SetColorAttachments(std::vector<VkFormat>& formats);
        void SetDepthFormat(VkFormat depthFormat);
        void DisableDepthTest();
        void EnableDepthTest(bool depthWriteEnable, VkCompareOp op);

        void EnableBlendingAdditive();
        void EnableBlendingAlphaBlend(size_t count = 1);
        void EnableMultiAlphaBlend(size_t count);
	};
}
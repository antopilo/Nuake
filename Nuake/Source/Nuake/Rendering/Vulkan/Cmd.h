#pragma once
#include <volk/volk.h>

#include "Nuake/Core/Core.h"
#include "Nuake/Rendering/Vulkan/VulkanImage/VulkanImage.h"

namespace Nuake
{
	class Cmd
	{
	private:
		VkCommandBuffer CmdBuffer;
		
	public:
		Cmd() = default;
		~Cmd() = default;

		Cmd(VkCommandBuffer cmdBuffer) : CmdBuffer(cmdBuffer) {}
		VkCommandBuffer GetCmdBuffer() { return CmdBuffer; }

	public:
		void BeginRendering(VkRenderingInfo renderInfo);
		void EndRendering();

		void BindPipeline(VkPipeline pipeline) const;
		void SetViewport(const Vector2& size) const;
		void SetScissor(const Vector2& size) const;
		void ClearColorImage(Ref<VulkanImage> img, Color color = Color(0, 0, 0, 1)) const;
		void BindDescriptorSet(VkPipelineLayout pipeline, VkDescriptorSet descriptor, uint32_t set) const;
		void BindIndexBuffer(VkBuffer buffer) const;
		void DrawIndexed(uint32_t count) const;
		void PushConstants(VkPipelineLayout pipeline, size_t size, void* data ) const;
		void CopyBuffer(VkBuffer src, VkBuffer dst, size_t size) const;
		void TransitionImageLayout(Ref<VulkanImage> img, VkImageLayout layout) const;
		void CopyImageToImage(Ref<VulkanImage> src, Ref<VulkanImage> dst) const;
	};
}
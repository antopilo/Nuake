#pragma once
#include <volk/volk.h>

#include "Nuake/Core/Core.h"
#include "Nuake/Rendering/Vulkan/VulkanImage/VulkanImage.h"

namespace Nuake
{
	class AllocatedBuffer;

	class Cmd
	{
	private:
		VkCommandBuffer CmdBuffer;
		
	public:
		Cmd() = default;
		~Cmd() = default;

		Cmd(VkCommandBuffer cmdBuffer) : CmdBuffer(cmdBuffer) {}
		VkCommandBuffer GetCmdBuffer() { return CmdBuffer; }

		class DebugMarkerScope
		{
		private:
			VkCommandBuffer cmd;

		public:
			DebugMarkerScope(VkCommandBuffer inCmd, const std::string& name, Color color = Color(1, 0, 0, 1)) :
				cmd(inCmd)
			{
				VkDebugUtilsLabelEXT labelInfo{};
				labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
				labelInfo.pLabelName = name.c_str();
				labelInfo.color[0] = color.r; // Red
				labelInfo.color[1] = color.g;
				labelInfo.color[2] = color.b;
				labelInfo.color[3] = color.a; // Alpha

				vkCmdBeginDebugUtilsLabelEXT(cmd, &labelInfo);
			}

			~DebugMarkerScope()
			{
				vkCmdEndDebugUtilsLabelEXT(cmd);
			}
		};

	public:
		void BeginRendering(VkRenderingInfo renderInfo);
		void EndRendering();

		DebugMarkerScope DebugScope(const std::string& name, Color color = Color(1, 0, 0, 1))
		{
			return DebugMarkerScope(CmdBuffer, name, color);
		}

		void DebugMarker(const std::string& name, Color color = Color(1, 0, 0, 1));
		void BindPipeline(VkPipeline pipeline) const;
		void BindSceneData(VkPipelineLayout pipelineLayout);
		void SetViewport(const Vector2& size) const;
		void SetScissor(const Vector2& size) const;
		void ClearColorImage(Ref<VulkanImage> img, Color color = Color(0, 0, 0, 1)) const;
		void ClearDepthImage(Ref<VulkanImage> img) const;
		void BindDescriptorSet(VkPipelineLayout pipeline, VkDescriptorSet descriptor, uint32_t set) const;
		void BindIndexBuffer(VkBuffer buffer) const;
		void DrawIndexed(uint32_t count) const;
		void PushConstants(VkPipelineLayout pipeline, size_t size, void* data ) const;
		void CopyBuffer(VkBuffer src, VkBuffer dst, size_t size) const;
		void CopyImageToBuffer(Ref<VulkanImage> src, Ref<AllocatedBuffer> dst, const Vector2& offset, const Vector3& extent = {1, 1, 1}) const;
		void TransitionImageLayout(Ref<VulkanImage> img, VkImageLayout layout) const;
		void CopyImageToImage(Ref<VulkanImage> src, Ref<VulkanImage> dst) const;

		void SetLineRasterizationMode(VkLineRasterizationMode mode) const;
		void SetLineWidth(float lineWidth) const;
		void SetLineStippleEnabled(bool enabled) const;
		void SetLineStipple(uint32_t factor, uint16_t pattern) const;
		void SetPrimitiveTopology(VkPrimitiveTopology topology) const;
		void SetPolygonMode(VkPolygonMode mode) const;
	};
}
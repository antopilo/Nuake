#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"
#include "src/Rendering/Vulkan/VulkanImage/VulkanImage.h"

#include <functional>
#include <vector>


namespace Nuake
{
	class Scene;

	struct PassRenderContext
	{
		Ref<Scene> scene;
		VkCommandBuffer commandBuffer;
	};

	class PassAttachment
	{
	public:
		std::string Name;
		ImageFormat Format;

	public:
		PassAttachment(const std::string& name, ImageFormat format);
		~PassAttachment() = default;
	};

	struct RenderPassSpec
	{
		std::string Name;
		bool DepthTest = true;
	};

	class RenderPass 
	{
	private:
		std::string Name;
		std::vector<PassAttachment> Attachments;
		std::vector<std::string> Inputs;

		std::function<void(PassRenderContext& ctx)> PreRender;
		std::function<void(PassRenderContext& ctx)> Render;
		std::function<void(PassRenderContext& ctx)> PostRender;

	public:
		RenderPass(const std::string& name);
		~RenderPass() = default;

	public:
		PassAttachment& AddAttachment(const std::string& name, ImageFormat format);
		void AddInput(const std::string& name);

		// Callbacks
		void SetPreRender(const std::function<void(PassRenderContext& ctx)>& func) { PreRender = func; }
		void SetRender(const std::function<void(PassRenderContext& ctx)>& func) { Render = func; }
		void SetPostRender(const std::function<void(PassRenderContext& ctx)>& func) { PostRender = func; }
	};

	class RenderPipeline
	{
	private:
		std::vector<RenderPass> RenderPasses;
		Vector2 Size;

	public:
		RenderPipeline(const Vector2& size);
		~RenderPipeline() = default;

	public:
		RenderPass& AddPass(const std::string& name);
	};
}
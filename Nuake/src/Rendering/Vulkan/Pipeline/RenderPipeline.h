#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"
#include "src/Rendering/Vulkan/VulkanImage/VulkanImage.h"

#include <functional>
#include <vector>
#include <span>
#include <src/Rendering/Vulkan/VulkanShader.h>


namespace Nuake
{
	class Scene;

	struct PassRenderContext
	{
		Ref<Scene> scene;
		VkCommandBuffer commandBuffer;
	};

	class TextureAttachment
	{
	public:
		std::string Name;
		ImageFormat Format;

	public:
		TextureAttachment(const std::string& name, ImageFormat format);
		TextureAttachment() = default;
		~TextureAttachment() = default;
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
		Ref<VulkanShader> VertShader;
		Ref<VulkanShader> FragShader;

		std::vector<TextureAttachment> Attachments;
		TextureAttachment DepthAttachment;
		std::vector<TextureAttachment> Inputs;

		std::function<void(PassRenderContext& ctx)> PreRender;
		std::function<void(PassRenderContext& ctx)> Render;
		std::function<void(PassRenderContext& ctx)> PostRender;

	public:
		RenderPass(const std::string& name);
		~RenderPass() = default;

	public:
		TextureAttachment& AddAttachment(const std::string& name, ImageFormat format, ImageUsage usage = ImageUsage::Default);
		void AddInput(const TextureAttachment& attachment);
		void SetShaders(Ref<VulkanShader> vertShader, Ref<VulkanShader> fragShader);
		void Build();

		// Callbacks
		void SetPreRender(const std::function<void(PassRenderContext& ctx)>& func) { PreRender = func; }
		void SetRender(const std::function<void(PassRenderContext& ctx)>& func) { Render = func; }
		void SetPostRender(const std::function<void(PassRenderContext& ctx)>& func) { PostRender = func; }
	};

	class RenderPipeline
	{
	private:
		bool Built;
		
		std::vector<RenderPass> RenderPasses;

	public:
		RenderPipeline();
		~RenderPipeline() = default;

	public:
		RenderPass& AddPass(const std::string& name);

		void Build();

		void Execute(std::span<std::string> inputs);
	};
}
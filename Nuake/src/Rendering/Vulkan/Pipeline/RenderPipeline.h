#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"
#include "src/Rendering/Vulkan/VulkanImage/VulkanImage.h"
#include "src/Rendering/Vulkan/VulkanShader.h"

#include <any>
#include <functional>
#include <span>
#include <vector>


namespace Nuake
{
	class Scene;
	class RenderPass;

	struct PassRenderContext
	{
		Ref<Scene> scene;
		VkCommandBuffer commandBuffer;
		Vector2 resolution;
		RenderPass* renderPass = nullptr;
	};

	class TextureAttachment
	{
	public:
		std::string Name;
		ImageFormat Format;
		Ref<VulkanImage> Image;

	public:
		TextureAttachment(const std::string& name, ImageFormat format, ImageUsage usage = ImageUsage::Default);
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
		std::vector<std::string> InputNames;
		std::map<std::string, TextureAttachment> Inputs;

		std::any PushConstant;
		size_t PushConstantSize;

		std::function<void(PassRenderContext& ctx)> PreRender;
		std::function<void(PassRenderContext& ctx)> RenderCb;
		std::function<void(PassRenderContext& ctx)> PostRender;

		// Vulkan structs

	public:
		VkPipeline Pipeline;
		VkPipelineLayout PipelineLayout;

	public:
		RenderPass(const std::string& name);
		~RenderPass() = default;

		void ClearAttachments(PassRenderContext& ctx);
		void TransitionAttachments(PassRenderContext& ctx);
		void UntransitionAttachments(PassRenderContext& ctx);
		void Render(PassRenderContext& ctx);

	public:
		std::string GetName() const { return Name; }
		TextureAttachment& AddAttachment(const std::string& name, ImageFormat format, ImageUsage usage = ImageUsage::Default);
		TextureAttachment& GetAttachment(const std::string& name);
		std::vector<TextureAttachment> GetAttachments();

		void AddInput(const std::string& name);
		std::vector<std::string> GetInputs();
		std::vector<TextureAttachment> GetInputAttachments();
		TextureAttachment& GetDepthAttachment() { return DepthAttachment; }

		void SetInput(const std::string& name, TextureAttachment attachment);
		void SetShaders(Ref<VulkanShader> vertShader, Ref<VulkanShader> fragShader);

		template<typename T>
		void SetPushConstant(T& pushConstant)
		{
			SetPushConstant(&pushConstant, sizeof(T));
		}

		void Build();

		// Callbacks
		void SetPreRender(const std::function<void(PassRenderContext& ctx)>& func) { PreRender = func; }
		void SetRender(const std::function<void(PassRenderContext& ctx)>& func) { RenderCb = func; }
		void SetPostRender(const std::function<void(PassRenderContext& ctx)>& func) { PostRender = func; }

	private:
		void SetPushConstant(std::any data, size_t size);

	};

	class RenderPipeline
	{
	private:
		bool Built;
		std::vector<RenderPass> RenderPasses;
		TextureAttachment FinalOutput;
	public:
		RenderPipeline();
		~RenderPipeline() = default;

	public:
		RenderPass& AddPass(const std::string& name);
		RenderPass& GetRenderPass(const std::string& name);

		bool Build();

		void Execute(PassRenderContext& ctx);
	};
}
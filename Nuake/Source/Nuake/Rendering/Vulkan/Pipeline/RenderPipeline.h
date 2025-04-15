#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Core/Maths.h"
#include "Nuake/Rendering/Vulkan/VulkanImage/VulkanImage.h"
#include "Nuake/Rendering/Vulkan/VulkanShader.h"
#include "Nuake/Rendering/Vulkan/Cmd.h"
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
		Cmd commandBuffer;
		Vector2 resolution;
		RenderPass* renderPass = nullptr;
		UUID cameraID;
		float selectedEntity;
		Color clearColor;
	};

	class TextureAttachment
	{
	public:
		std::string Name;
		ImageFormat Format;
		Ref<VulkanImage> Image;
		bool ClearOnLoad = true;

	public:
		TextureAttachment(const std::string& name, ImageFormat format, ImageUsage usage = ImageUsage::Default, bool clearOnLoad = true);
		TextureAttachment() = default;
		~TextureAttachment() = default;
	};

	struct RenderPassSpec
	{
		std::string Name;
		bool DepthTest = true;
	};

	using PassAttachments = std::vector<Ref<VulkanImage>>;
	
	enum class PolygonTopology
	{
		POINT_LIST = 0,
		LINE_LIST = 1,
		LINE_STRIP = 2,
		TRIANGLE_LIST = 3,
		TRIANGLE_STRIP = 4,
		TRIANGLE_FAN = 5,
		LINE_LIST_WITH_ADJACENCY = 6,
		LINE_STRIP_WITH_ADJACENCY = 7,
		TRIANGLE_LIST_WITH_ADJACENCY = 8,
		TRIANGLE_STRIP_WITH_ADJACENCY = 9,
		PATCH_LIST = 10,
	};

	class RenderPass 
	{
	private:
		std::string Name;
		bool HasDepthTest = false;
		Ref<VulkanShader> VertShader;
		Ref<VulkanShader> FragShader;

		std::vector<TextureAttachment> Attachments;
		TextureAttachment DepthAttachment;
		std::vector<std::string> InputNames;
		std::map<std::string, TextureAttachment> Inputs;

		PolygonTopology Topology;
		std::any PushConstant;
		size_t PushConstantSize;

		Color ClearColor;

		bool IsLinePass;

		std::function<void(PassRenderContext& ctx)> PreRender;
		std::function<void(PassRenderContext& ctx)> RenderCb;
		std::function<void(PassRenderContext& ctx)> PostRender;
	public:
		VkPipeline Pipeline;
		VkPipelineLayout PipelineLayout;

	public:
		RenderPass(const std::string& name);
		~RenderPass() = default;

		void Execute(PassRenderContext& ctx, PassAttachments& inputs);

	private:
		void Render(PassRenderContext& ctx, PassAttachments& inputs);
		void ClearAttachments(PassRenderContext& ctx, PassAttachments& inputs);
		void TransitionAttachments(PassRenderContext& ctx, PassAttachments& inputs);
		void UntransitionAttachments(PassRenderContext& ctx, PassAttachments& inputs);

	public:
		void SetDepthTest(bool enabled) { HasDepthTest = enabled; }
		std::string GetName() const { return Name; }
		TextureAttachment& AddAttachment(const std::string& name, ImageFormat format, ImageUsage usage = ImageUsage::Default, bool clearOnLoad = true);
		TextureAttachment& GetAttachment(const std::string& name);
		std::vector<TextureAttachment> GetAttachments();

		void AddInput(const std::string& name);
		std::vector<std::string> GetInputs();
		std::vector<TextureAttachment> GetInputAttachments();
		TextureAttachment& GetDepthAttachment() { return DepthAttachment; }

		void SetInput(const std::string& name, TextureAttachment attachment);
		void SetShaders(Ref<VulkanShader> vertShader, Ref<VulkanShader> fragShader);

		void SetTopology(PolygonTopology topology);
		void SetClearColor(const Color& color);

		template<typename T>
		void SetPushConstant(T& pushConstant)
		{
			SetPushConstant(&pushConstant, sizeof(T));
		}

		void SetIsLinePass(bool enabled);

		void Build();

		// Callbacks
		void SetPreRender(const std::function<void(PassRenderContext& ctx)>& func) { PreRender = func; }
		void SetRender(const std::function<void(PassRenderContext& ctx)>& func) { RenderCb = func; }
		void SetPostRender(const std::function<void(PassRenderContext& ctx)>& func) { PostRender = func; }

	private:
		void SetPushConstant(std::any data, size_t size);

	};

	using PipelineAttachments = std::vector<PassAttachments>;

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
		RenderPass& GetRenderPass(const std::string& name);

		bool Build();
		void Execute(PassRenderContext& ctx, PipelineAttachments& inputs);
	};
}
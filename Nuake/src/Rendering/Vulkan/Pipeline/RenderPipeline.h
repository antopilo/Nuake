#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"

#include "src/Rendering/Vulkan/VulkanImage/VulkanImage.h"

#include <vector>


namespace Nuake
{
	class PassAttachment
	{
	public:
		std::string Name;
		ImageFormat Format;

	public:
		PassAttachment(const std::string& name, ImageFormat format);
		~PassAttachment() = default;

	};

	class RenderPass 
	{
	private:
		std::string Name;
		std::vector<PassAttachment> Attachments;
		std::vector<std::string> Inputs;

	public:
		RenderPass(const std::string& name);
		~RenderPass() = default;

	public:
		PassAttachment& AddAttachment(const std::string& name, ImageFormat format);
		void AddInput(const std::string& name);
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
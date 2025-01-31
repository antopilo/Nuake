#pragma once

#include <string>
#include <vector>
#include "VulkanImage/VulkanImage.h"


namespace Nuake
{
	// Create Pipeline
	// Pipeline is a graph or pass that connects with dependencies
	// Bind framebuffer or StartRendering on a Pass
	// 1. Transition all images
	// 2. Create render info attachment info
	// 3. BeginRendering
	// 3.5 Bind pipeline & descriptor sets
	// 4. EndRendering

	// A render pass has inputs & outputs

	struct RenderContext
	{
		int cmd;
	};

	struct ImageTexture
	{
		std::string name;
		ImageFormat format;
	};

	struct RenderPassInput
	{
		std::string name;
		ImageFormat image;
	};

	struct RenderPassOutput
	{
		std::string name;
	};

	struct RenderPassInfo
	{
		std::string name;
		std::vector<RenderPassInput> attachments;
		std::vector<RenderPassOutput> outputs;

		std::function<void(RenderContext& ctx)> BeginRender;
	};

	RenderPassInfo gBuffer = {
		"GBuffer",
		{	// Output
			{ "albedo", ImageFormat::RGBA8 },
			{ "normal", ImageFormat::RGBA8 },
			{ "material", ImageFormat::RGBA8 }
		},
		{	// Input
			{ "materialAlbedo" }
		}, 
		[](RenderContext& ctx) // On Begin
		{

		}
	};

	RenderPassInfo shading = {
		"Shading",
		{	// Output
			{ "output", ImageFormat::RGBA16F }
		},
		{	// Input
			{ "albedo" },
			{ "normal" },
			{ "material" }
		}
	};


	class RenderPass
	{
	public:
		RenderPass();
		~RenderPass();

	private:
		
	};
}
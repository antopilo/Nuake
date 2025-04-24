#include "SceneViewport.h"

#include "Nuake/Rendering/Vulkan/VulkanImage/VulkanImage.h"
#include "VulkanRenderer.h"

#include "SceneRenderPipeline.h"

using namespace Nuake;

Viewport::Viewport(UUID inViewId, const Vector2& inViewportSize) :
	id(UUID()),
	viewportSize(inViewportSize),
	queuedResize(inViewportSize),
	viewId(inViewId)
{
	renderTarget = CreateRef<VulkanImage>(ImageFormat::RGBA16F, viewportSize);
}

int Viewport::MousePick(const Vector2& mouseCoord)
{


	return 0;
}

bool Viewport::Resize()
{
	if (viewportSize != queuedResize)
	{
		viewportSize = queuedResize;

		auto newRenderTarget = CreateRef<VulkanImage>(ImageFormat::RGBA16F, viewportSize);
		VkRenderer::Get().ImmediateSubmit([&](VkCommandBuffer cmd) 
		{
			Cmd command(cmd);
			command.TransitionImageLayout(renderTarget, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
			command.TransitionImageLayout(newRenderTarget, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			command.CopyImageToImage(renderTarget, newRenderTarget);
			command.TransitionImageLayout(renderTarget, VK_IMAGE_LAYOUT_GENERAL);
			command.TransitionImageLayout(newRenderTarget, VK_IMAGE_LAYOUT_GENERAL);
		});

		renderTarget = newRenderTarget;

		return true;
	}

	return false;
}


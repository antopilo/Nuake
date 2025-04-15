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

		renderTarget = CreateRef<VulkanImage>(ImageFormat::RGBA16F, viewportSize);
		renderTarget->GetImGuiDescriptorSet();
		return true;
	}

	return false;
}


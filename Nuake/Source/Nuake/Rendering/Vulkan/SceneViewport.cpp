#include "SceneViewport.h"

#include "Nuake/Rendering/Vulkan/VulkanImage/VulkanImage.h"

using namespace Nuake;

Viewport::Viewport(UUID inViewId, const Vector2& inViewportSize) :
	id(UUID()),
	viewportSize(inViewportSize),
	queuedResize(inViewportSize),
	viewId(inViewId)
{
	renderTarget = CreateRef<VulkanImage>(ImageFormat::RGBA16F, viewportSize);
}

bool Viewport::Resize()
{
	if (viewportSize != queuedResize)
	{
		viewportSize = queuedResize;

		renderTarget = CreateRef<VulkanImage>(ImageFormat::RGBA16F, viewportSize);
		return true;
	}

	return false;
}


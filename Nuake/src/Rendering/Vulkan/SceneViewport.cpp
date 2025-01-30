#include "SceneViewport.h"

#include "src/Rendering/Vulkan/VulkanImage/VulkanImage.h"

using namespace Nuake;

Viewport::Viewport(UUID inViewId, const Vector2& inViewportSize) :
	id(UUID()),
	viewportSize(inViewportSize),
	viewId(inViewId)
{
	renderTarget = CreateRef<VulkanImage>(ImageFormat::RGBA16F, viewportSize);
}

bool Viewport::Resize()
{
	if (renderTarget->GetSize() != viewportSize)
	{
		renderTarget = CreateRef<VulkanImage>(ImageFormat::RGBA16F, viewportSize);
		return true;
	}

	return false;
}


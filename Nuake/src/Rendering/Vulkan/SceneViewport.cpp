#include "SceneViewport.h"

#include "src/Rendering/Vulkan/VulkanImage/VulkanImage.h"

using namespace Nuake;

Viewport::Viewport(UUID inViewId, const Vector2& inViewportSize) :
	id(UUID()),
	viewportSize(inViewportSize),
	viewId(inViewId)
{
	// Create render target
	renderTarget = CreateRef<VulkanImage>(ImageFormat::RGBA8, viewportSize);
}


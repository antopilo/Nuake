#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Resource/UUID.h"
#include "Nuake/Rendering/Vulkan/Cmd.h"

#include <volk/volk.h>

namespace Nuake
{
	class Scene;
	class VulkanImage;

	struct RenderContext
	{
		Ref<Scene> CurrentScene;		// The scene we are rendering	
		Cmd CommandBuffer;  // The command buffer we are recording into
		Vector2 Size;
		UUID CameraID;
		Ref<VulkanImage> ViewportImage;
		int SelectedEntityID = -1;
		// We might add more to this!
	};
}
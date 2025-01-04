#pragma once
#include "src/Core/Core.h"

#include <volk/volk.h>

namespace Nuake
{
	class Scene;

	struct RenderContext
	{
		Ref<Scene> CurrentScene;		// The scene we are rendering	
		VkCommandBuffer CommandBuffer;  // The command buffer we are recording into
		// ...
		// We might add more to this!
	};
}
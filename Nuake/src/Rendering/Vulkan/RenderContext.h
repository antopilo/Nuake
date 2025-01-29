#pragma once
#include "src/Core/Core.h"
#include "src/Resource/UUID.h"
#include "src/Rendering/Vulkan/Cmd.h"

#include <volk/volk.h>

namespace Nuake
{
	class Scene;

	struct RenderContext
	{
		Ref<Scene> CurrentScene;		// The scene we are rendering	
		Cmd CommandBuffer;  // The command buffer we are recording into
		Vector2 Size;
		UUID CameraID;
		// We might add more to this!
	};
}
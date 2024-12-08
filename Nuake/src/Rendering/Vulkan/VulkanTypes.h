#pragma once

#include <src/Core/Maths.h>
#include "src/Rendering/Vulkan/VulkanAllocatedBuffer.h"


namespace Nuake
{
	struct Vertex
	{
		Vector3 position;
		float uv_x;
		Vector3 normal;
		float uv_y;
		Vector4 color;
	};

	// holds the resources needed for a mesh
	struct GPUMeshBuffers 
	{
		AllocatedBuffer indexBuffer;
		AllocatedBuffer vertexBuffer;
		VkDeviceAddress vertexBufferAddress;
	};

	// push constants for our mesh object draws
	struct GPUDrawPushConstants
	{
		Matrix4 worldMatrix;
		VkDeviceAddress vertexBuffer;
	};
}

#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"
#include "src/Resource/UUID.h"


namespace Nuake
{
	class VulkanImage;

	class Viewport
	{
	private:
		UUID id;
		Vector2 queuedResize;
		Vector2 viewportSize;

		UUID viewId;
		Ref<VulkanImage> renderTarget;

	public:
		Viewport(UUID viewId, const Vector2& size);
		~Viewport() = default;

	public:
		UUID GetID() const { return id; }
		UUID GetViewID() const { return viewId; }

		const Vector2& GetViewportSize() const { return viewportSize; }
		void SetViewportSize(const Vector2& size)
		{
			viewportSize = size;
		}

		void QueueResize(const Vector2& inSize)
		{
			queuedResize = inSize;
		}

		Ref<VulkanImage> GetRenderTarget() const { return renderTarget; }
		bool Resize();
	};
}
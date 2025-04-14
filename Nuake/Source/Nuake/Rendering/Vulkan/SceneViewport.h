#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Core/Maths.h"
#include "Nuake/Core/MulticastDelegate.h"
#include "Nuake/Resource/UUID.h"


namespace Nuake
{
	class VulkanImage;
	class DebugCmd;
	class DebugLineCmd;

	class Viewport
	{
	private:
		UUID id;
		Vector2 queuedResize;
		Vector2 viewportSize;

		UUID viewId;
		Ref<VulkanImage> renderTarget;
		MulticastDelegate<DebugCmd&> debugDrawDelegate;
		MulticastDelegate<DebugLineCmd&> debugLineDrawDelegate;

		int selectedEntityID;
	public:
		Viewport(UUID viewId, const Vector2& size);
		~Viewport() = default;

	public:
		UUID GetID() const { return id; }
		UUID GetViewID() const { return viewId; }
		void SetViewID(UUID inViewID) { this->viewId = inViewID; }
		const Vector2& GetViewportSize() const { return viewportSize; }
		void SetViewportSize(const Vector2& size)
		{
			viewportSize = size;
		}

		bool QueueResize(const Vector2& inSize)
		{
			queuedResize = inSize;

			return queuedResize != viewportSize;
		}

		int MousePick(const Vector2& mouseCoord);

		void SetSelectedEntityID(int id) { selectedEntityID = id; }
		int GetSelectedEntityID() const { return selectedEntityID; }

		Ref<VulkanImage> GetRenderTarget() const { return renderTarget; }
		bool Resize();

		void OnDebugDraw(DebugCmd& debugCmd)
		{
			debugDrawDelegate.Broadcast(debugCmd);
		}

		void OnLineDraw(DebugLineCmd& debugCmd)
		{
			debugLineDrawDelegate.Broadcast(debugCmd);
		}

		MulticastDelegate<DebugCmd&>& GetOnDebugDraw() 
		{
			return debugDrawDelegate;
		}

		MulticastDelegate<DebugLineCmd&>& GetOnLineDraw()
		{
			return debugLineDrawDelegate;
		}
	};
}
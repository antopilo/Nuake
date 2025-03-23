#pragma once
#include "Cmd.h"

#include "Nuake/Rendering/Vulkan/Constant/DebugConstant.h"

namespace Nuake
{
	class Scene;
	class PassRenderContext;

	// This is the API to render anything debugging related
	// Like lines, shapes, gizmos, etc.
	class DebugCmd
	{
	private:
		Cmd& cmd;
		PassRenderContext& ctx;
		DebugConstant debugConstant;

	public:
		DebugCmd(Cmd& inCmd, PassRenderContext& inCtx);
		~DebugCmd() = default;

	public:
		Ref<Scene> GetScene() const;

		void DrawQuad(const Matrix4& transform);
		void DrawLine(const Vector3& start, const Vector3& end, const Color& color) const;
		void DrawSphere(const Vector2& position, float radius, const Color& color) const;
		void DrawCube(const Vector3& position, const Vector3& size, const Color& color) const;
		void DrawAABB(const Vector3& min, const Vector3& max, const Color& color) const;
	};
}
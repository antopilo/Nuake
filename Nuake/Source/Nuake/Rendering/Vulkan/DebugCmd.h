#pragma once
#include "Cmd.h"

#include "Nuake/Rendering/Vulkan/Constant/DebugConstant.h"
#include "Nuake/Rendering/Vulkan/Constant/LineConstant.h"

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
		void DrawTexturedQuad(const Matrix4& transform, Ref<VulkanImage> texture, const Color& color = Color(1, 1, 1 ,1), int32_t entityId = -1);

		void DrawSphere(const Vector2& position, float radius, const Color& color) const;
		void DrawCube(const Vector3& position, const Vector3& size, const Color& color) const;
		void DrawAABB(const Vector3& min, const Vector3& max, const Color& color) const;
	};

	struct DrawLineRequest
	{
		Vector3 Start;
		Vector3 End;
		Color Color;
	};

	class DebugLineCmd
	{
	private:
		Cmd& cmd;
		PassRenderContext& ctx;
		LineConstant lineConstant;

	public:
		DebugLineCmd(Cmd& inCmd, PassRenderContext& inCtx);
		~DebugLineCmd() = default;

		Ref<Scene> GetScene() const;

		void DrawLine(const Matrix4& transform, const Color& color, float lineWidth = 1.0f);
		void DrawBox(const Matrix4& transform, const Color& color, float lineWidth = 1.0f, bool stippled = false);
		void DrawCapsule(const Matrix4& transform, const Color& color, float lineWidth = 1.0f, bool stippled = false);
		void DrawSphere(const Matrix4& transform, const Color& color, float lineWidth = 1.0f, bool stippled = false);
		void DrawCylinder(const Matrix4& transform, const Color& color, float lineWidth = 1.0f, bool stippled = false);
		void DrawArrow(const Vector3& from, const Vector3& to, const Matrix4& view, const Matrix4& proj, const Color& color, float lineWidth = 1.0f, bool stippled = false);
		void DrawCone(const Matrix4& transform, const Color& color, float lineWidth = 1.0f, bool stippled = false);
	};
}
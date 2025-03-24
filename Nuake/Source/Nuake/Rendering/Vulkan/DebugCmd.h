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
		void DrawTexturedQuad(const Matrix4& transform, Ref<VulkanImage> texture);

		void DrawSphere(const Vector2& position, float radius, const Color& color) const;
		void DrawCube(const Vector3& position, const Vector3& size, const Color& color) const;
		void DrawAABB(const Vector3& min, const Vector3& max, const Color& color) const;
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
	};
}
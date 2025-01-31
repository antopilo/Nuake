#pragma once
#include "Cmd.h"

namespace Nuake
{
	// This is the API to render anything debugging related
	// Like lines, shapes, gizmos, etc.
	class DebugCmd
	{
	private:
		Cmd& cmd;

	public:
		DebugCmd(Cmd& inCmd);
		~DebugCmd() = default;

	public:
		void DrawLine(const Vector3& start, const Vector3& end, const Color& color) const;
		void DrawSphere(const Vector2& position, float radius, const Color& color) const;
		void DrawCube(const Vector3& position, const Vector3& size, const Color& color) const;
		void DrawAABB(const Vector3& min, const Vector3& max, const Color& color) const;
	};
}
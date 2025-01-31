#include "DebugCmd.h"


using namespace Nuake;

DebugCmd::DebugCmd(Cmd& inCmd) : 
	cmd(inCmd) 
{
}

void DebugCmd::DrawLine(const Vector3& start, const Vector3& end, const Color& color) const
{
	
}

void DebugCmd::DrawSphere(const Vector2& position, float radius, const Color& color) const
{
}

void DebugCmd::DrawCube(const Vector3& position, const Vector3& size, const Color& color) const
{
}

void DebugCmd::DrawAABB(const Vector3& min, const Vector3& max, const Color& color) const
{
}
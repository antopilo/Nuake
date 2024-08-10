#include "EngineNetAPI.h"
#include <Coral/String.hpp>
#include <src/Core/Maths.h>
#include <Engine.h>
#include "src/Rendering/SceneRenderer.h"

namespace Nuake {

	void Log(Coral::String string)
	{
		Logger::Log(string, ".net", VERBOSE);
	}

	void DrawLine(float startX, float startY, float startZ, float endX, float endY, float endZ, float r, float g, float b, float a, float life)
	{
		const Vector3 start = { startX, startY, startZ };
		const Vector3 end = { endX, endY, endZ };
		const Vector4 color = { r, g, b, a };

		Engine::GetCurrentScene()->m_SceneRenderer->DrawDebugLine(start, end, color, life);
	}

	void EngineNetAPI::RegisterMethods()
	{
		RegisterMethod("Engine.LoggerLogIcall", (void*)(&Log));
		RegisterMethod("Debug.DrawLineIcall", &DrawLine);
	}

}

#include "EngineNetAPI.h"
#include <Coral/String.hpp>
#include <src/Core/Maths.h>
#include <Engine.h>
#include "src/Rendering/SceneRenderer.h"
#include <Coral/Array.hpp>
#include <src/Physics/PhysicsManager.h>

namespace Nuake {

	struct CapsuleInternal
	{
		float Radius;
		float Height;
	};

	struct BoxInternal
	{
		float x;
		float y;
		float z;
	};

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

	Coral::Array<float> ConvertHitsToArray(const std::vector<ShapeCastResult> hits)
	{
		std::vector<float> results;
		for (const auto& hit : hits)
		{
			results.push_back(hit.ImpactPosition.x);
			results.push_back(hit.ImpactPosition.y);
			results.push_back(hit.ImpactPosition.z);
			results.push_back(hit.ImpactNormal.x);
			results.push_back(hit.ImpactNormal.y);
			results.push_back(hit.ImpactNormal.z);
			results.push_back(hit.Fraction);

		}
		return Coral::Array<float>::New(results);
	}

	Coral::Array<float> ShapeCastCapsule(float fromX, float fromY, float fromZ, float toX, float toY, float toZ, CapsuleInternal capsuleInternal)
	{
		auto capsule = CreateRef<Physics::Capsule>(capsuleInternal.Radius, capsuleInternal.Height);
		const Vector3 from = { fromX, fromY, fromZ };
		const Vector3 to = { toX, toY, toZ };
		auto hits = PhysicsManager::Get().Shapecast(from, to, capsule);

		return ConvertHitsToArray(hits);
	}

	Coral::Array<float> ShapeCastBox(float fromX, float fromY, float fromZ, float toX, float toY, float toZ, BoxInternal boxInternal)
	{
		auto capsule = CreateRef<Physics::Box>(boxInternal.x, boxInternal.y, boxInternal.z);
		const Vector3 from = { fromX, fromY, fromZ };
		const Vector3 to = { toX, toY, toZ };
		auto hits = PhysicsManager::Get().Shapecast(from, to, capsule);

		return ConvertHitsToArray(hits);
	}

	void EngineNetAPI::RegisterMethods()
	{
		RegisterMethod("Engine.LoggerLogIcall", (void*)(&Log));
		RegisterMethod("Debug.DrawLineIcall", &DrawLine);

		RegisterMethod("Physic.ShapeCastCapsuleIcall", &ShapeCastCapsule);
		RegisterMethod("Physic.ShapeCastBoxIcall", &ShapeCastBox);
	}

}

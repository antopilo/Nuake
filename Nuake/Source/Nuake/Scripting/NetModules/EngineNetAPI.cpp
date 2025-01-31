#include "EngineNetAPI.h"

#include "Nuake/Core/Maths.h"
#include "Nuake/Rendering/SceneRenderer.h"
#include "Engine.h"
#include "Nuake/Physics/PhysicsManager.h"

#include <Coral/String.hpp>
#include <Coral/ManagedObject.hpp>
#include <Coral/Array.hpp>
#include "Coral/Type.hpp"
#include "..\..\Subsystems\EngineSubsystemScriptable.h"

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

	void DrawShapeBox(Vector3 position, Quat rotation, Vector3 box, Vector4 color, float life, float width)
	{
		Engine::GetCurrentScene()->m_SceneRenderer->DrawDebugShape(position, rotation, CreateRef<Physics::Box>(box.x, box.y, box.z), color, life, width);
	}

	void DrawShapeSphere(Vector3 position, Quat rotation, float radius, Vector4 color, float life, float width)
	{
		Engine::GetCurrentScene()->m_SceneRenderer->DrawDebugShape(position, rotation, CreateRef<Physics::Sphere>(radius), color, life, width);
	}

	void DrawShapeCylinder(Vector3 position, Quat rotation, float radius, float height, Vector4 color, float life, float width)
	{
		Engine::GetCurrentScene()->m_SceneRenderer->DrawDebugShape(position, rotation, CreateRef<Physics::Cylinder>(radius, height), color, life, width);
	}

	void DrawShapeCapsule(Vector3 position, Quat rotation, float radius, float height, Vector4 color, float life, float width)
	{
		Engine::GetCurrentScene()->m_SceneRenderer->DrawDebugShape(position, rotation, CreateRef<Physics::Capsule>(radius, height), color, life, width);
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
			results.push_back(hit.Layer);
			results.push_back(hit.EntityID);

		}
		return Coral::Array<float>::New(results);
	}

	Coral::Array<float> Raycast(float fromX, float fromY, float fromZ, float toX, float toY, float toZ)
	{
		const Vector3 from = { fromX, fromY, fromZ };
		const Vector3 to = { toX, toY, toZ };
		auto hits = PhysicsManager::Get().Raycast(from, to);
		return ConvertHitsToArray(hits);
	}

	Coral::Array<float> ShapeCastCapsule(float fromX, float fromY, float fromZ, float toX, float toY, float toZ, CapsuleInternal capsuleInternal)
	{
		auto capsule = CreateRef<Physics::Capsule>(capsuleInternal.Radius, capsuleInternal.Height);
		const Vector3 from = { fromX, fromY, fromZ };
		const Vector3 to = { toX, toY, toZ };
		auto hits = PhysicsManager::Get().Shapecast(from, to, capsule);

		return ConvertHitsToArray(hits);
	}

	Coral::Array<float> ShapeCastSphere(float fromX, float fromY, float fromZ, float toX, float toY, float toZ, float radius)
	{
		auto sphere = CreateRef<Physics::Sphere>(radius);
		const Vector3 from = { fromX, fromY, fromZ };
		const Vector3 to = { toX, toY, toZ };
		auto hits = PhysicsManager::Get().Shapecast(from, to, sphere);

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

	void LoadScene(Coral::String path)
	{
		if (!FileSystem::FileExists(path))
		{
			Logger::Log("Failed to load scene with path: " + std::string(path), ".net/scene", CRITICAL);
			return;
		}

		Engine::QueueSceneSwitch(std::string(path));
	}

	Coral::ManagedObject GetEngineSubsystemByName(Coral::String subsystemName)
	{
		const Ref<EngineSubsystemScriptable> scriptedSubsystem = Engine::GetScriptedSubsystem(subsystemName);
		if (scriptedSubsystem == nullptr)
		{
			return {};
		}
		
		return scriptedSubsystem->GetManagedObjectInstance();
	}

	void EngineNetAPI::RegisterMethods()
	{
		RegisterMethod("Engine.LoadSceneIcall", &LoadScene);
		RegisterMethod("Engine.LoggerLogIcall", (void*)(&Log));
		RegisterMethod("Engine.GetSubsystemByNameIcall", &GetEngineSubsystemByName);

		// Debug renderer
		RegisterMethod("Debug.DrawLineIcall", &DrawLine);
		RegisterMethod("Debug.DrawShapeBoxIcall", &DrawShapeBox);
		RegisterMethod("Debug.DrawShapeSphereIcall", &DrawShapeSphere);
		RegisterMethod("Debug.DrawShapeCapsuleIcall", &DrawShapeCapsule);
		RegisterMethod("Debug.DrawShapeCylinderIcall", &DrawShapeCylinder);

		RegisterMethod("Physic.RayCastIcall", &Raycast);
		RegisterMethod("Physic.ShapeCastSphereIcall", &ShapeCastSphere);
		RegisterMethod("Physic.ShapeCastCapsuleIcall", &ShapeCastCapsule);
		RegisterMethod("Physic.ShapeCastBoxIcall", &ShapeCastBox);
	}

}

#include "PhysicsManager.h"
#include "PhysicsShapes.h"
#include "../Core/Core.h"

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>


namespace Nuake
{
	PhysicsManager* PhysicsManager::m_Instance;

	void PhysicsManager::RegisterBody(Ref<Physics::RigidBody> rb) 
	{
		m_World->AddRigidbody(rb);
	}

	void PhysicsManager::RegisterCharacterController(Ref<Physics::CharacterController> cc) 
	{
		m_World->AddCharacterController(cc);
	}

	void PhysicsManager::SetBodyTransform(const Entity& entity, const Vector3& position, const Quat& rotation)
	{
		m_World->SetBodyPosition(entity, position, rotation);
	}

	void PhysicsManager::SetCharacterControllerPosition(const Entity& entity, const Vector3& position)
	{
		m_World->SetCharacterControllerPosition(entity, position);
	}

	void PhysicsManager::Step(Timestep ts)
	{
		m_World->StepSimulation(ts);
	}

	void PhysicsManager::Reset()
	{
		m_World->Clear();
	}

	std::vector<ShapeCastResult> PhysicsManager::Raycast(const Vector3& from, const Vector3&  to)
	{
		return m_World->Raycast(from, to);
	}

	std::vector<ShapeCastResult> PhysicsManager::Shapecast(const Vector3& from, const Vector3&  to, const Ref<Physics::PhysicShape>& shape)
	{
		return m_World->CastShape(from, to, shape);
	}

	const std::vector<Physics::CollisionData> PhysicsManager::GetCollisions()
	{
		return m_World->GetCollisionsData();
	}

	void PhysicsManager::DrawDebug()
	{
		if (m_DrawDebug)
			m_World->DrawDebug();
	}

	void PhysicsManager::Init() 
	{
		JPH::RegisterDefaultAllocator();

		JPH::Factory::sInstance = new JPH::Factory();
		JPH::RegisterTypes();

		m_World = CreateRef<Physics::DynamicWorld>();
		m_World->SetGravity(Vector3(0, -3, 0));

		m_IsRunning = false;
	}
}

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

	void PhysicsManager::RegisterGhostBody(Ref<GhostObject> rb)
	{
		m_World->AddGhostbody(rb);
	}

	void PhysicsManager::RegisterCharacterController(Ref<Physics::CharacterController> cc) 
	{
		m_World->AddCharacterController(cc);
	}

	void PhysicsManager::Step(Timestep ts)
	{
		m_World->StepSimulation(ts);
	}

	void PhysicsManager::Reset()
	{
		m_World->Clear();
	}

	RaycastResult PhysicsManager::Raycast(const Vector3& from, const Vector3&  to)
	{
		return m_World->Raycast(from, to);
	}

	void PhysicsManager::DrawDebug()
	{
		if (m_DrawDebug)
			m_World->DrawDebug();
	}

	void PhysicsManager::Init() 
	{
		Logger::Log("Initializing Jolt physics.");
		JPH::RegisterDefaultAllocator();

		Logger::Log("Creating factory & registering types.");
		JPH::Factory::sInstance = new JPH::Factory();
		JPH::RegisterTypes();

		m_World = new Physics::DynamicWorld();
		m_World->SetGravity(Vector3(0, -3, 0));

		m_IsRunning = false;
	}
}

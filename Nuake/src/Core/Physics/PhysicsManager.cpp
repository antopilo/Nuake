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

	RaycastResult PhysicsManager::Raycast(glm::vec3 from, glm::vec3 to)
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
		JPH::RegisterDefaultAllocator();

		JPH::Factory::sInstance = new JPH::Factory();

		JPH::RegisterTypes();

		// This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
		// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
		const uint32_t cMaxBodies = 1024;

		// This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
		const uint32_t cNumBodyMutexes = 0;

		// This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
		// body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
		// too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
		// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
		const uint32_t cMaxBodyPairs = 1024;

		// This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
		// number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
		// Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
		const uint32_t cMaxContactConstraints = 1024;


		m_World = new Physics::DynamicWorld();
		m_World->SetGravity(glm::vec3(0, -3, 0));


		m_IsRunning = false;
	}
}

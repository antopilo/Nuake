#include "PhysicsManager.h"
#include "PhysicsShapes.h"
#include "btBulletDynamicsCommon.h"
#include "../Core/Core.h"
PhysicsManager* PhysicsManager::m_Instance;

void PhysicsManager::RegisterBody(Ref<Physics::RigidBody> rb) {
	m_World->AddRigidbody(rb);
}

void PhysicsManager::RegisterCharacterController(Ref<Physics::CharacterController> cc) {
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
	btVector3 btFrom(from.x, from.y, from.z);
	btVector3 btTo(to.x, to.y, to.z);
	btCollisionWorld::ClosestRayResultCallback res(btFrom, btTo);

	return m_World->Raycast(from, to);

}

void PhysicsManager::DrawDebug()
{
	if(m_DrawDebug)
		m_World->DrawDebug();
}

void PhysicsManager::Init() {

	m_World = new Physics::DynamicWorld();
	m_World->SetGravity(glm::vec3(0, -3, 0));
	m_IsRunning = false;
}
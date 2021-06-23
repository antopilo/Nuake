#include "PhysicsSystem.h"
#include "src/Scene/Scene.h"
#include "src/Scene/Components/BoxCollider.h"

#include <src/Scene/Components/RigidbodyComponent.h>
#include "src/Scene/Entities/Entity.h"
#include <src/Core/Physics/PhysicsManager.h>
#include <src/Scene/Components/CharacterControllerComponent.h>
#include <src/Scene/Components/QuakeMap.h>
#include <src/Scene/Components/BSPBrushComponent.h>
PhysicsSystem::PhysicsSystem(Scene* scene) 
{
    m_Scene = scene;
}

void PhysicsSystem::Init()
{
    // Create physic world.
	auto view = m_Scene->m_Registry.view<TransformComponent, RigidBodyComponent>();
	for (auto e : view)
	{
		auto [transform, rigidbody] = view.get<TransformComponent, RigidBodyComponent>(e);
		Entity ent = Entity({ e, m_Scene });
		
		if (ent.HasComponent<BoxColliderComponent>())
		{
			float mass = rigidbody.mass;
            
			BoxColliderComponent& boxComponent = ent.GetComponent<BoxColliderComponent>();
			Ref<Physics::Box> boxShape = CreateRef<Physics::Box>(boxComponent.Size);
            
			Ref<Physics::RigidBody> btRigidbody = CreateRef<Physics::RigidBody>(mass, transform.Translation, boxShape);
			rigidbody.m_Rigidbody = btRigidbody;
            
			btRigidbody->SetKinematic(rigidbody.IsKinematic);
            
			PhysicsManager::Get()->RegisterBody(btRigidbody);
		}
	}

	// character controllers
	auto ccview = m_Scene->m_Registry.view<TransformComponent, CharacterControllerComponent>();
	for (auto e : ccview)
	{
		auto [transform, cc] = ccview.get<TransformComponent, CharacterControllerComponent>(e);

		cc.CharacterController = CreateRef<Physics::CharacterController>(cc.Height, cc.Radius, cc.Mass, transform.Translation);
		Entity ent = Entity({ e, m_Scene });

		PhysicsManager::Get()->RegisterCharacterController(cc.CharacterController);
	}

	auto bspView = m_Scene->m_Registry.view<TransformComponent, BSPBrushComponent>();
	for (auto e : bspView)
	{
		auto [transform, brush] = bspView.get<TransformComponent, BSPBrushComponent>(e);

		if (brush.IsSolid)
		{
			for (auto m : brush.Meshes)
			{
				Ref<Physics::MeshShape> meshShape = CreateRef<Physics::MeshShape>(m);
				Ref<Physics::RigidBody> btRigidbody = CreateRef<Physics::RigidBody>(0.0f, transform.GlobalTranslation, meshShape);
				PhysicsManager::Get()->RegisterBody(btRigidbody);
			}
		}
	}
}

void PhysicsSystem::Update(Timestep ts)
{
	// Update rigidbodies
	PhysicsManager::Get()->Step(ts);

	auto physicGroup = m_Scene->m_Registry.view<TransformComponent, RigidBodyComponent>();
	for (auto e : physicGroup) {
		auto [transform, rb] = physicGroup.get<TransformComponent, RigidBodyComponent>(e);
		rb.SyncTransformComponent(&m_Scene->m_Registry.get<TransformComponent>(e));
	}

	auto ccGroup = m_Scene->m_Registry.view<TransformComponent, CharacterControllerComponent>();
	for (auto e : ccGroup) {
		auto [transform, rb] = ccGroup.get<TransformComponent, CharacterControllerComponent>(e);
		rb.SyncWithTransform(m_Scene->m_Registry.get<TransformComponent>(e));
	}
}

void PhysicsSystem::FixedUpdate(Timestep ts)
{
    
}

void PhysicsSystem::Exit()
{
	PhysicsManager::Get()->Reset();
    
}
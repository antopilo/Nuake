#include "PhysicsSystem.h"
#include "src/Scene/Scene.h"
#include "src/Scene/Components/BoxCollider.h"

#include <src/Scene/Components/RigidbodyComponent.h>
#include "src/Scene/Entities/Entity.h"
#include <src/Core/Physics/PhysicsManager.h>
#include <src/Scene/Components/CharacterControllerComponent.h>
#include <src/Scene/Components/QuakeMap.h>
#include <src/Scene/Components/BSPBrushComponent.h>
#include <src/Scene/Components/TriggerZone.h>

namespace Nuake
{
	PhysicsSystem::PhysicsSystem(Scene* scene)
	{
		m_Scene = scene;
	}

	bool PhysicsSystem::Init()
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
			cc.CharacterController->SetEntity(ent);

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
					btRigidbody->SetEntityID(Entity{ e, m_Scene });
					brush.Rigidbody.push_back(btRigidbody);
					PhysicsManager::Get()->RegisterBody(btRigidbody);
				}
			}
		}

		auto bspTriggerView = m_Scene->m_Registry.view<TransformComponent, BSPBrushComponent, TriggerZone>();
		for (auto e : bspTriggerView) {
			auto [transform, brush, trigger] = bspTriggerView.get<TransformComponent, BSPBrushComponent, TriggerZone>(e);

			Ref<Physics::MeshShape> meshShape = CreateRef<Physics::MeshShape>(brush.Meshes[0]);
			Ref<GhostObject> ghostBody = CreateRef<GhostObject>(transform.GlobalTranslation, meshShape);
			trigger.GhostObject = ghostBody;

			PhysicsManager::Get()->RegisterGhostBody(ghostBody);
		}
		return true;
	}

	void PhysicsSystem::Update(Timestep ts)
	{
		PhysicsManager::Get()->Step(ts);

		auto brushes = m_Scene->m_Registry.view<TransformComponent, BSPBrushComponent>();
		for (auto e : brushes) {
			auto [transform, brush] = brushes.get<TransformComponent, BSPBrushComponent>(e);

			for (auto& r : brush.Rigidbody) {
				r->m_Transform->setOrigin(btVector3(transform.GlobalTranslation.x, transform.GlobalTranslation.y, transform.GlobalTranslation.z));
				r->UpdateTransform(*r->m_Transform);
			}

			if (!brush.IsFunc)
				continue;

			brush.Targets.clear();
			auto targetnameView = m_Scene->m_Registry.view<TransformComponent, NameComponent>();
			for (auto e2 : targetnameView) {
				auto [ttransform, name] = targetnameView.get<TransformComponent, NameComponent>(e2);

				if (name.Name == brush.target) {
					brush.Targets.push_back(Entity{ e2, m_Scene });
				}
			}
		}


		auto bspTriggerView = m_Scene->m_Registry.view<TransformComponent, BSPBrushComponent, TriggerZone>();
		for (auto e : bspTriggerView) {
			auto [transform, brush, trigger] = bspTriggerView.get<TransformComponent, BSPBrushComponent, TriggerZone>(e);
			trigger.GhostObject->ScanOverlap();

			brush.Targets.clear();
			auto targetnameView = m_Scene->m_Registry.view<TransformComponent, NameComponent>();
			for (auto e2 : targetnameView) {
				auto [ttransform, name] = targetnameView.get<TransformComponent, NameComponent>(e2);

				if (name.Name == brush.target) {
					brush.Targets.push_back(Entity{ e2, m_Scene });
				}
			}
		}

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
}

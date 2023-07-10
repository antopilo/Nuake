#include "TransformSystem.h"

#include "src/Core/Maths.h"
#include "src/Scene/Scene.h"
#include <src/Scene/Components/TransformComponent.h>
#include <src/Scene/Components/CameraComponent.h>
#include <src/Scene/Components/ParentComponent.h>

namespace Nuake 
{
	TransformSystem::TransformSystem(Scene* scene)
	{
		m_Scene = scene;
	}

	bool TransformSystem::Init()
	{
		UpdateTransform();
		return true;
	}

	void TransformSystem::Update(Timestep ts)
	{
		UpdateTransform();
	}

	void TransformSystem::FixedUpdate(Timestep ts)
	{

	}

	void TransformSystem::Exit()
	{

	}

	void TransformSystem::UpdateTransform()
	{
		// Calculate all local transforms
		auto localTransformView = m_Scene->m_Registry.view<TransformComponent>();
		for (auto tv : localTransformView)
		{
			TransformComponent& transform = localTransformView.get<TransformComponent>(tv);
			if (transform.Dirty)
			{
				Matrix4 localTransform = Matrix4(1.0f);
				auto& localTranslate = transform.GetLocalPosition();
				auto& localRot = transform.GetLocalRotation();
				auto& localScale = transform.GetLocalScale();

				localTransform = glm::translate(localTransform, localTranslate);
				localTransform = localTransform * glm::toMat4(localRot);
				localTransform = glm::scale(localTransform, localScale);

				transform.SetLocalTransform(localTransform);
				transform.Dirty = false;
			}
		}

		// Calculate all global transforms
		auto transformView = m_Scene->m_Registry.view<ParentComponent, TransformComponent>();
		for (auto e : transformView) 
		{
			auto [parent, transform] = transformView.get<ParentComponent, TransformComponent>(e);

			if (!parent.HasParent)
			{
				// If no parents, then globalTransform is local transform.
				transform.SetGlobalTransform(transform.GetLocalTransform());
				transform.SetGlobalPosition(transform.GetLocalPosition());
				transform.SetGlobalRotation(transform.GetLocalRotation());
				transform.SetGlobalScale(transform.GetLocalScale());
				continue;
			}

			Entity currentParent = Entity((entt::entity)e, m_Scene);

			Matrix4 globalTransform = transform.GetLocalTransform();
			Vector3 globalPosition = transform.GetLocalPosition();
			Quat globalOrientation = transform.GetLocalRotation();
			Vector3 globalScale = transform.GetLocalScale();

			ParentComponent parentComponent = currentParent.GetComponent<ParentComponent>();
			while (parentComponent.HasParent)
			{
				TransformComponent& transformComponent = parentComponent.Parent.GetComponent<TransformComponent>();

				globalTransform = transformComponent.GetLocalTransform() * globalTransform;

				globalPosition += transformComponent.GetLocalPosition();
				globalOrientation *= transformComponent.GetLocalRotation();
				globalScale *= transformComponent.GetLocalScale();
		
				NameComponent& nameComponent = parentComponent.Parent.GetComponent<NameComponent>();

				parentComponent = parentComponent.Parent.GetComponent<ParentComponent>();
			}
				
			transform.SetGlobalPosition(globalPosition);
			transform.SetGlobalRotation(globalOrientation);
			transform.SetGlobalScale(globalScale);
			transform.SetGlobalTransform(globalTransform);
		}

		auto camView = m_Scene->m_Registry.view<TransformComponent, CameraComponent>();
		for (auto& e : camView)
		{
			auto [transform, camera] = camView.get<TransformComponent, CameraComponent>(e);
			Matrix4 cameraTransform = camera.CameraInstance->GetTransformRotation();
			camera.CameraInstance->Translation = transform.GlobalTranslation;

			const auto& globalRotation = transform.GetGlobalRotation();
			const auto& forward = QuatToDirection(globalRotation);
			camera.CameraInstance->SetDirection(forward);
		}
	}
}

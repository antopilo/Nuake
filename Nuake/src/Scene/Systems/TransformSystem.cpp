#include "TransformSystem.h"

#include "src/Core/Maths.h"

#include "src/Scene/Scene.h"
#include <src/Scene/Components/TransformComponent.h>
#include <src/Scene/Components/CameraComponent.h>
#include <src/Scene/Components/ParentComponent.h>

namespace Nuake {
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
		auto camView = m_Scene->m_Registry.view<TransformComponent, CameraComponent>();
		for (auto e : camView)
		{
			auto [transform, camera] = camView.get<TransformComponent, CameraComponent>(e);
			Matrix4 cameraTransform = camera.CameraInstance->GetTransformRotation();
		}

		auto transformView = m_Scene->m_Registry.view<ParentComponent, TransformComponent>();
		for (auto e : transformView) 
		{
			auto [parent, transform] = transformView.get<ParentComponent, TransformComponent>(e);
			Entity currentParent = Entity((entt::entity)e, m_Scene);

			Vector3 globalPosition = Vector3();
			Vector3 globalRotation = Vector3();
			Vector3 globalScale = Vector3();
			if (parent.HasParent)
			{
				ParentComponent& parentComponent = currentParent.GetComponent<ParentComponent>();
				while (parentComponent.HasParent)
				{
					TransformComponent& transformComponent = parentComponent.Parent.GetComponent<TransformComponent>();
					globalPosition += transformComponent.Translation;
					globalRotation += transformComponent.Rotation;
					globalScale *= transformComponent.Scale;

					parentComponent = parentComponent.Parent.GetComponent<ParentComponent>();
				}
				
				transform.GlobalTranslation = globalPosition + transform.Translation;
				transform.GlobalRotation = globalRotation + transform.Rotation;
				transform.GlobalScale = globalScale * transform.Scale;
			}
			else
			{
				transform.GlobalTranslation = transform.Translation;
				transform.GlobalRotation = transform.Rotation;
				transform.Scale = transform.Scale;
			}
		}
	}
}

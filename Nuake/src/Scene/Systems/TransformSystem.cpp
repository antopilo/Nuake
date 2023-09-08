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
				const Vector3& localTranslate = transform.GetLocalPosition();
				const Quat& localRot = glm::normalize(transform.GetLocalRotation());
				const Vector3& localScale = transform.GetLocalScale();
				const Matrix4& translationMatrix = glm::translate(Matrix4(1.0f), localTranslate);
				const Matrix4& rotationMatrix = glm::mat4_cast(localRot);
				const Matrix4& scaleMatrix = glm::scale(Matrix4(1.0f), localScale);
				const Matrix4& newLocalTransform = translationMatrix * rotationMatrix * scaleMatrix;

				transform.SetLocalTransform(newLocalTransform);
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

#ifndef FRAME_PERFECT_TRANSFORM
			ParentComponent parentComponent = currentParent.GetComponent<ParentComponent>();
			if (parentComponent.HasParent)
			{
				TransformComponent& transformComponent = parentComponent.Parent.GetComponent<TransformComponent>();

				globalPosition = transformComponent.GetGlobalPosition() + (globalPosition);

				globalScale *= transformComponent.GetGlobalScale();
				globalOrientation = transformComponent.GetGlobalRotation() * globalOrientation;
				globalTransform = transformComponent.GetGlobalTransform() * globalTransform;
			}
#else
			while (parentComponent.HasParent)
			{
				TransformComponent& transformComponent = parentComponent.Parent.GetComponent<TransformComponent>();
			
				globalPosition = transformComponent.GetLocalPosition() + (globalPosition);
			
				globalScale *= transformComponent.GetLocalScale();
				globalOrientation = transformComponent.GetLocalRotation() * globalOrientation;
				globalTransform = transformComponent.GetLocalTransform() * globalTransform;
			
				NameComponent& nameComponent = parentComponent.Parent.GetComponent<NameComponent>();
				parentComponent = parentComponent.Parent.GetComponent<ParentComponent>();
			}
#endif // FRAME_PERFECT_TRANSFORM

			transform.SetGlobalPosition(globalPosition);
			transform.SetGlobalRotation(globalOrientation);
			transform.SetGlobalScale(globalScale);
			transform.SetGlobalTransform(globalTransform);
		}

		auto camView = m_Scene->m_Registry.view<TransformComponent, CameraComponent>();
		for (auto& e : camView)
		{
			auto [transform, camera] = camView.get<TransformComponent, CameraComponent>(e);
			const Matrix4& cameraTransform = camera.CameraInstance->GetTransformRotation();

			camera.CameraInstance->Translation = transform.GlobalTranslation;

			const Quat& globalRotation = transform.GetGlobalRotation();
			const Matrix4& translationMatrix = glm::translate(Matrix4(1.0f), transform.GetGlobalPosition());
			const Matrix4& rotationMatrix = glm::mat4_cast(globalRotation);
			const Vector4& forward = Vector4(0, 0, -1, 1);
			const Vector4& globalForward = rotationMatrix * forward;

			const Vector4& right = Vector4(1, 0, 0, 1);
			const Vector4& globalRight = rotationMatrix * right;
			camera.CameraInstance->Direction = globalForward;
			camera.CameraInstance->Right = globalRight;
;			camera.CameraInstance->SetTransform(glm::inverse(translationMatrix * rotationMatrix));
		}
	}
}

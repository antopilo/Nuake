#pragma once
#include "RigidbodyComponent.h"

#include "src/Core/Physics/Rigibody.h"
#include "src/Core/Physics/PhysicsManager.h"
#include "src/Rendering/Renderer.h"

namespace Nuake {
	RigidBodyComponent::RigidBodyComponent()
	{
		//m_Rigidbody = CreateRef<Physics::RigidBody>();
	}

	Ref<Physics::RigidBody> RigidBodyComponent::GetRigidBody() const
	{
		return m_Rigidbody;
	}

	void RigidBodyComponent::SetRigidBody(Ref<Physics::RigidBody> rb)
	{
		m_Rigidbody = rb;
		PhysicsManager::Get()->RegisterBody(rb);
	}

	bool RigidBodyComponent::HasRigidBody() const
	{
		return m_Rigidbody != nullptr;
	}

	float RigidBodyComponent::GetMass() {
		if (m_Rigidbody)
			return m_Rigidbody->GetMass();
		return 0.0f;
	}

	void RigidBodyComponent::SetMass(float m)
	{
		if (!m_Rigidbody)
			return;
		m_Rigidbody->SetMass(m);
	}


	void RigidBodyComponent::SyncTransformComponent(TransformComponent* tc)
	{
		if (!m_Rigidbody)
			return;

		glm::vec3 newPosition = m_Rigidbody->GetPosition();
		glm::vec3 newRotation = m_Rigidbody->GetRotation();
		tc->Translation = newPosition;
		//tc->Rotation = newRotation;
	}

	void RigidBodyComponent::SyncWithTransform(TransformComponent* tc)
	{
		if (!m_Rigidbody)
			return;

		btTransform newTransform;
		newTransform.setIdentity();
		newTransform.setOrigin(btVector3(tc->Translation.x, tc->Translation.t, tc->Translation.z));

		btQuaternion quat;
		//quat.setEulerZYX(tc->Rotation.x, tc->Rotation.y, tc->Rotation.z);
		newTransform.setRotation(quat);
		m_Rigidbody->UpdateTransform(newTransform);
	}

	void RigidBodyComponent::DrawShape(TransformComponent* tc)
	{

	}


	void RigidBodyComponent::DrawEditor() {

	}
}

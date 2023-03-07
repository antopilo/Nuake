#pragma once
#include "RigidbodyComponent.h"

#include "src/Core/Physics/Rigibody.h"
#include "src/Core/Physics/PhysicsManager.h"
#include "src/Rendering/Renderer.h"

namespace Nuake {
	RigidBodyComponent::RigidBodyComponent() : Mass(0.0f)
	{
		//m_Rigidbody = CreateRef<Physics::RigidBody>();
	}

	Ref<Physics::RigidBody> RigidBodyComponent::GetRigidBody() const
	{
		return m_Rigidbody;
	}

	void RigidBodyComponent::SyncTransformComponent(TransformComponent* tc)
	{
		if (!m_Rigidbody)
			return;

	}

	void RigidBodyComponent::SyncWithTransform(TransformComponent* tc)
	{
		if (!m_Rigidbody)
			return;


	}

	void RigidBodyComponent::DrawShape(TransformComponent* tc)
	{

	}


	void RigidBodyComponent::DrawEditor() {

	}
}

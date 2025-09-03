#pragma once
#include "RigidbodyComponent.h"

#include "Nuake/Physics/Rigibody.h"
#include "Nuake/Physics/PhysicsManager.h"
#include "Nuake/Rendering/Renderer.h"

namespace Nuake {
	RigidBodyComponent::RigidBodyComponent() : Mass(0.0f)
	{
		//m_Rigidbody = CreateRef<Physics::RigidBody>();
	}

	Ref<Physics::RigidBody> RigidBodyComponent::GetRigidBody() const
	{
		return Rigidbody;
	}

	void RigidBodyComponent::SyncTransformComponent(TransformComponent* tc)
	{
		if (!GetRigidBody())
			return;
	}

	void RigidBodyComponent::SyncWithTransform(TransformComponent* tc)
	{
		if (!GetRigidBody())
			return;
	}

	void RigidBodyComponent::DrawShape(TransformComponent* tc)
	{

	}

	void RigidBodyComponent::DrawEditor() {

	}
}

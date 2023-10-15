#pragma once
#include "RigidbodyComponent.h"

#include "src/Physics/Rigibody.h"
#include "src/Physics/PhysicsManager.h"
#include "src/Rendering/Renderer.h"

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

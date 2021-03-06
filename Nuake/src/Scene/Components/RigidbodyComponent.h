#pragma once
#include "TransformComponent.h"
#include "BaseComponent.h"
#include "src/Core/Core.h"

namespace Nuake {
	namespace Physics 
	{
		class RigidBody;
	};

	class RigidBodyComponent
	{
	public:
		float mass = 0.0f;
		Ref<Physics::RigidBody> m_Rigidbody;
		bool IsKinematic = false;

		RigidBodyComponent();
		Ref<Physics::RigidBody> GetRigidBody() const;
		void SetRigidBody(Ref<Physics::RigidBody> rb);
		bool HasRigidBody() const;

		void SetMass(float m);
		float GetMass();

		void SyncTransformComponent(TransformComponent* tc);
		void SyncWithTransform(TransformComponent* tc);


		void DrawShape(TransformComponent* tc);
		void DrawEditor();
	};
}

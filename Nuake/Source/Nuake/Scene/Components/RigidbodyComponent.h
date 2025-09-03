#pragma once

#include "Component.h"

#include "TransformComponent.h"
#include "VisibilityComponent.h"
#include "Nuake/Core/Core.h"

namespace Nuake {
	namespace Physics 
	{
		class RigidBody;
	};

	class RigidBodyComponent : public Component
	{
		NUAKECOMPONENT(RigidBodyComponent, "Rigid Body")

		static void InitializeComponentClass()
		{
			BindComponentField<&RigidBodyComponent::Mass>("Mass", "Mass");
			BindComponentField<&RigidBodyComponent::LockX>("LockX", "Lock X");
			BindComponentField<&RigidBodyComponent::LockY>("LockY", "Lock Y");
			BindComponentField<&RigidBodyComponent::LockZ>("LockZ", "Lock Z");
		}

	public:
		float Mass;
		bool LockX = false;
		bool LockY = false;
		bool LockZ = false;
		bool HasBeenMoved = false;
		Ref<Physics::RigidBody> Rigidbody;

		Vector3 QueuedForce = Vector3();

		RigidBodyComponent();
		Ref<Physics::RigidBody> GetRigidBody() const;
		
		void SyncTransformComponent(TransformComponent* tc);
		void SyncWithTransform(TransformComponent* tc);

		void DrawShape(TransformComponent* tc);
		void DrawEditor();


		json Serialize()
		{
			BEGIN_SERIALIZE();
			SERIALIZE_VAL(Mass);
			SERIALIZE_VAL(LockX);
			SERIALIZE_VAL(LockY);
			SERIALIZE_VAL(LockZ);
			END_SERIALIZE();
		}

		bool Deserialize(const json& j)
		{
			DESERIALIZE_VAL(Mass);
			DESERIALIZE_VAL(LockX);
			DESERIALIZE_VAL(LockY);
			DESERIALIZE_VAL(LockZ);

			return true;
		}
	};
}

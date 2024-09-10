#pragma once

#include "src/Core/Object/Object.h"
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
		NUAKECOMPONENT(RigidBodyComponent, "Rigid Body")

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
			SERIALIZE_VAL_LBL("Mass", Mass);
			SERIALIZE_VAL(LockX);
			SERIALIZE_VAL(LockY);
			SERIALIZE_VAL(LockZ);
			END_SERIALIZE();
		}

		bool Deserialize(const json& j)
		{
			Mass = j["Mass"];

			if (j.contains("LockX"))
			{
				LockX = j["LockX"];
			}

			if (j.contains("LockY"))
			{
				LockY = j["LockY"];
			}

			if (j.contains("LockZ"))
			{
				LockZ = j["LockZ"];
			}

			return true;
		}
	};
}

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
		float Mass;
		Ref<Physics::RigidBody> Rigidbody;

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
			END_SERIALIZE();
		}

		bool Deserialize(std::string str)
		{
			BEGIN_DESERIALIZE();
			Mass = j["Mass"];
			return true;
		}
	};
}

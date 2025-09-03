#pragma once

#include "Component.h"

#include "Nuake/Physics/PhysicsShapes.h"
#include "Nuake/Core/Core.h"

namespace Nuake
{
	class SphereColliderComponent : public Component
	{
		NUAKECOMPONENT(SphereColliderComponent, "Sphere Component")

		static void InitializeComponentClass()
		{
			BindComponentField<&SphereColliderComponent::IsTrigger>("IsTrigger", "Is Trigger");
			BindComponentProperty<&SphereColliderComponent::SetRadius, &SphereColliderComponent::GetRadius>("Radius", "Radius");
		}

	public:
		Ref<Physics::PhysicShape> Sphere;

		float Radius = 0.5f;
		bool IsTrigger = false;

		void SetRadius(const float newRadius)
		{
			Radius = newRadius;
		}

		float GetRadius()
		{
			return Radius;
		}

		json Serialize()
		{
			BEGIN_SERIALIZE();
			j["Radius"] = Radius;
			j["IsTrigger"] = IsTrigger;
			END_SERIALIZE();
		}

		bool Deserialize(const json& j)
		{
			this->Radius = j["Radius"];
			this->IsTrigger = j["IsTrigger"];
			return true;
		}
	};
}

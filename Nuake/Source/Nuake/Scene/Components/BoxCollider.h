#pragma once

#include "Component.h"

#include "Nuake/Physics/PhysicsShapes.h"
#include "Nuake/Core/Core.h"

namespace Nuake
{
    class BoxColliderComponent : public Component
    {
        NUAKECOMPONENT(BoxColliderComponent, "Box Collider")

        static void InitializeComponentClass()
        {
            BindComponentField<&BoxColliderComponent::IsTrigger>("IsTrigger", "Is Trigger");
            BindComponentProperty<&BoxColliderComponent::SetSize, &BoxColliderComponent::GetSize>("Size", "Size");
        }

    public:
        Ref<Physics::PhysicShape> Box;
        Vector3 Size = Vector3(0.5f, 0.5f, 0.5f);
        bool IsTrigger = true;

        void SetSize(const Vector3& newSize)
        {
            Size = newSize;
        }

        Vector3 GetSize()
        {
            return Size;
        }

        json Serialize();
        bool Deserialize(const json& j);
    };
}

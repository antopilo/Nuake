#pragma once

#include "Component.h"

#include "src/Physics/PhysicsShapes.h"
#include "src/Core/Core.h"

namespace Nuake
{
    class BoxColliderComponent : public Component
    {
        NUAKECOMPONENT(BoxColliderComponent, "Box Collider")

        static void InitializeComponentClass()
        {
            BINDCOMPONENTFIELD(BoxColliderComponent::IsTrigger, "Is Trigger");

            ComponentFactory
                .data<&BoxColliderComponent::SetSize, &BoxColliderComponent::GetSize>(entt::hashed_string("size"))
                .prop(HashedName::DisplayName, "Size");
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

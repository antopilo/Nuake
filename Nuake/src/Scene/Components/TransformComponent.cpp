#pragma once
#include "TransformComponent.h"

namespace Nuake
{
    TransformComponent::TransformComponent()
    {
        GlobalTranslation = Vector3(0, 0, 0);
        Translation = Vector3(0, 0, 0);
        Rotation = Vector3(0, 0, 0);
        Scale = Vector3(1, 1, 1);
    }

    Matrix4 TransformComponent::GetGlobalTransform() const
    {
        return GlobalTransform;
    }

    Matrix4 TransformComponent::GetLocalTransform() const
    {
        return LocalTransform;
    }
}

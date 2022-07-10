#pragma once
#include "TransformComponent.h"

namespace Nuake
{
    TransformComponent::TransformComponent()
    {
        GlobalTranslation = Vector3(0, 0, 0);
        Translation = Vector3(0, 0, 0);
        Orientation = Quat(0, 0, 0, 0);
        GlobalOrientation = Orientation;
        //Rotation = Vector3(0, 0, 0);
        Scale = Vector3(1, 1, 1);
    }

    void TransformComponent::SetRotation(float x, float y, float z)
    {
		Quat QuatAroundX = Quat(1.0, 0.0, 0.0, x);
		Quat QuatAroundY = Quat(0.0, 1.0, 0.0, y);
		Quat QuatAroundZ = Quat(0.0, 0.0, 1.0, z);
        Quat finalOrientation = QuatAroundX * QuatAroundY * QuatAroundZ;
        Orientation = finalOrientation;
    }

    Matrix4 TransformComponent::GetGlobalTransform() const
    {
        return LocalTransform;
    }

    Matrix4 TransformComponent::GetLocalTransform() const
    {
        return LocalTransform;
    }
}

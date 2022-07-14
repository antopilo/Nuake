#pragma once
#include "TransformComponent.h"

namespace Nuake
{
    TransformComponent::TransformComponent()
    {
        GlobalTranslation = Vector3(0, 0, 0);
        Translation = Vector3(0, 0, 0);

        Rotation = Quat(0, 0, 0, 1);
        GlobalRotation = Rotation;

        GlobalScale = Vector3(1, 1, 1);
        Scale = GlobalScale;

        LocalTransform = Matrix4(1);
        GlobalTransform = Matrix4(1);
    }

    void TransformComponent::SetLocalRotation(const Quat& quat)
    {
        Rotation = quat;
        Dirty = true;
    }

    Quat TransformComponent::GetLocalRotation() const
    {
        return Rotation;
    }

    Quat TransformComponent::GetGlobalRotation() const
    {
        return GlobalRotation;
    }

    void TransformComponent::SetGlobalRotation(const Quat& rotation)
    {
        GlobalRotation = rotation;
    }

    Vector3 TransformComponent::GetLocalPosition() const
    {
        return Translation;
    }

    void TransformComponent::SetLocalPosition(const Vector3& position)
    {
        Translation = position;
        Dirty = true;
    }

    Vector3 TransformComponent::GetGlobalPosition() const
    {
        return GlobalTranslation;
    }

    void TransformComponent::SetGlobalPosition(const Vector3& position)
    {
        GlobalTranslation = position;
    }

    Vector3 TransformComponent::GetLocalScale() const
    {
        return Scale;
    }

    void TransformComponent::SetLocalScale(const Vector3& scale)
    {
        Scale = scale;
        Dirty = true;
    }

    Vector3 TransformComponent::GetGlobalScale() const
    {
        return GlobalScale;
    }

    void TransformComponent::SetGlobalScale(const Vector3& scale)
    {
        GlobalScale = scale;
    }

    Matrix4 TransformComponent::GetGlobalTransform() const
    {
        return GlobalTransform;
    }

    void TransformComponent::SetGlobalTransform(const Matrix4& transform)
    {
        GlobalTransform = transform;
    }

    Matrix4 TransformComponent::GetLocalTransform() const
    {
        return LocalTransform;
    }

    void TransformComponent::SetLocalTransform(const Matrix4& transform)
    {
        LocalTransform = transform;
    }
}

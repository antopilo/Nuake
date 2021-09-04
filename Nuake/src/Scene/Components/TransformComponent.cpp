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

    glm::mat4 TransformComponent::GetTransform()
    {
        //Matrix4 transform = Matrix4(1.0f);
        //transform = glm::translate(transform, GlobalTranslation);
        //transform = glm::rotate(transform, glm::radians(Rotation.x), Vector3(1, 0, 0));
        //transform = glm::rotate(transform, glm::radians(Rotation.y), Vector3(0, 1, 0));
        //transform = glm::rotate(transform, glm::radians(Rotation.z), Vector3(0, 0, 1));
        //transform = glm::scale(transform, Scale);
        return Transform;
    }
}

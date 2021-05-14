#pragma once
#include "TransformComponent.h"

#include <glm\ext\matrix_transform.hpp>
#include "../ImGuiHelper.h"

TransformComponent::TransformComponent() {
    Translation = glm::vec3(0, 0, 0);
    Rotation = glm::vec3(0, 0, 0);
    Scale = glm::vec3(1, 1, 1);
}

glm::mat4 TransformComponent::GetTransform() {
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, Translation);
    transform = glm::rotate(transform, glm::radians(Rotation.x), glm::vec3(1, 0, 0));
    transform = glm::rotate(transform, glm::radians(Rotation.y), glm::vec3(0, 1, 0));
    transform = glm::rotate(transform, glm::radians(Rotation.z), glm::vec3(0, 0, 1));
    transform = glm::scale(transform, Scale);
    return transform;
}

void TransformComponent::DrawEditor()
{
    ImGuiHelper::DrawVec3("Translation", &Translation);
    ImGuiHelper::DrawVec3("Rotation", &Rotation);
    ImGuiHelper::DrawVec3("Scale", &Scale);
    ImGui::Separator();
}
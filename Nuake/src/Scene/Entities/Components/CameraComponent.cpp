#pragma once
#include "CameraComponent.h"
#include "../ImGuiHelper.h"

CameraComponent::CameraComponent()
{
    CameraInstance = CreateRef<Camera>();
}

void CameraComponent::DrawEditor() {
    ImGui::Text("Camera");
    ImGui::SliderFloat("Exposure", &CameraInstance->Exposure, 0.0f, 2.0f, "%.2f", 1.0f);
    ImGui::SliderFloat("FOV", &CameraInstance->Fov, 1.0f, 180.0f, "%.2f", 1.0f);
    ImGui::SliderFloat("Speed", &CameraInstance->Speed, 0.1f, 5.0f, "%.2f", 1.0f);


}
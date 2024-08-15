#pragma once
#include "CameraComponent.h"
#include "src/Rendering/Camera.h"
#include "src/Scene/Entities/ImGuiHelper.h"

namespace Nuake {
    CameraComponent::CameraComponent()
    {
        CameraInstance = CreateRef<Camera>();
    }

}

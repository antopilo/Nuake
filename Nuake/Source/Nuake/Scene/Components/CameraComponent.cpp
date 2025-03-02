#include "CameraComponent.h"
#include "Nuake/Rendering/Camera.h"
#include "Nuake/Scene/Entities/ImGuiHelper.h"

namespace Nuake {
    CameraComponent::CameraComponent()
    {
        CameraInstance = CreateRef<Camera>();
    }

}

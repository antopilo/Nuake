#include "src/Resource/ModelLoader.h"
#include "MeshComponent.h"
#include "src/Rendering/Mesh/Mesh.h"
#include "src/Rendering/Textures/TextureManager.h"
#include "src/Rendering/Textures/Material.h"
#include "src/Rendering/Renderer.h"
#include <src/Rendering/Textures/MaterialManager.h>

namespace Nuake {
    MeshComponent::MeshComponent()
    {

    }

    void MeshComponent::LoadModel()
    {
        auto loader = ModelLoader();
        this->ModelResource = loader.LoadModel(ModelPath);
    }
}


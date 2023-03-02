#include "src/Resource/ModelLoader.h"
#include "ModelComponent.h"
#include "src/Rendering/Mesh/Mesh.h"
#include "src/Rendering/Textures/TextureManager.h"
#include "src/Rendering/Textures/Material.h"
#include "src/Rendering/Renderer.h"
#include <src/Rendering/Textures/MaterialManager.h>

namespace Nuake {
    ModelComponent::ModelComponent()
    {

    }

    void ModelComponent::LoadModel()
    {
        auto loader = ModelLoader();
        this->ModelResource = loader.LoadModel(FileSystem::Root + ModelPath);
    }
}


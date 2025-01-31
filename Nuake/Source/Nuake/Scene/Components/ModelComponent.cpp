#include "Nuake/Resource/ModelLoader.h"
#include "ModelComponent.h"
#include "Nuake/Rendering/Mesh/Mesh.h"
#include "Nuake/Rendering/Textures/TextureManager.h"
#include "Nuake/Rendering/Textures/Material.h"
#include "Nuake/Rendering/Renderer.h"
#include "Nuake/Rendering/Textures/MaterialManager.h"

namespace Nuake {

    ModelComponent::ModelComponent()
    {

    }

    void ModelComponent::LoadModel()
    {
        auto loader = ModelLoader();
        //this->ModelResource = loader.LoadModel(ModelPath);
    }
}


#pragma once

#include "src/Core/String.h"
#include "src/Resource/Model.h"
#include "src/Resource/ResourceLoader.h"
#include "src/Resource/Serializable.h"

#include <string>
#include <vector>


namespace Nuake
{
    struct ModelComponent
    {
        Ref<Model> ModelResource;
        std::string ModelPath;

        ModelComponent();

        void LoadModel();
       
        std::string directory;

        json Serialize() 
        {
            BEGIN_SERIALIZE();
            SERIALIZE_VAL(ModelPath);
            if (ModelResource)
            {
                SERIALIZE_OBJECT(ModelResource);
            }
            END_SERIALIZE();
        }

        bool Deserialize(const json& j) 
        {
            ModelPath = j["ModelPath"];
            ModelResource = CreateRef<Model>();

            if (ModelPath.empty() || !String::EndsWith(ModelPath, ".mesh"))
            {
                if (j.contains("ModelResource"))
                {
                    auto& res = j["ModelResource"];
                    ModelResource->Deserialize(res);
                }
            }
            else
            {
                ModelResource = ResourceLoader::LoadModel(ModelPath);
            }

            return true;
        }
    };
}

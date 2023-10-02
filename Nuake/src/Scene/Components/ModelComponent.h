#pragma once
#include <vector>

#include "src/Resource/Serializable.h"
#include "src/Resource/Model.h"

#include <string>

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

            if (j.contains("ModelResource"))
            {
                auto& res = j["ModelResource"];
                ModelResource->Deserialize(res);
            }

            return true;
        }
    };
}

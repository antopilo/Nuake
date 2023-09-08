#pragma once
#include <vector>

#include "src/Resource/Serializable.h"
#include "src/Resource/SkinnedModel.h"

#include <string>


namespace Nuake
{
    struct SkinnedModelComponent
    {
        Ref<SkinnedModel> ModelResource;
        std::string ModelPath;

        SkinnedModelComponent();

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
            ModelResource = CreateRef<SkinnedModel>();

            if (j.contains("ModelResource"))
            {
                auto& res = j["ModelResource"];
                ModelResource->Deserialize(res);
            }

            return true;
        }
    };
}

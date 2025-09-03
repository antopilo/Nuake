#pragma once

#include "Component.h"
#include "Nuake/Resource/RID.h"
#include "Nuake/Core/String.h"
#include "Nuake/Resource/Model.h"
#include "Nuake/Resource/ResourceLoader.h"
#include "Nuake/Resource/Serializable.h"

#include <string>
#include <vector>
#include "Nuake/Resource/Serializer/BinarySerializer.h"


namespace Nuake
{
    struct ModelComponent : public Component
    {
        NUAKECOMPONENT(ModelComponent, "Model")

        RID ModelResource;
        //Ref<Model> ModelResource;
        std::string ModelPath;

        ModelComponent();

        void LoadModel();
       
        std::string directory;

        json Serialize() 
        {
            BEGIN_SERIALIZE();
            SERIALIZE_VAL(ModelPath);
            SERIALIZE_RID(ModelResource, ModelResource.ID);
            //if (ModelResource)
            //{
            //    BinarySerializer serializer;
            //    serializer.SerializeModel(FileSystem::RelativeToAbsolute("model.nkmesh"), ModelResource);
            //    SERIALIZE_OBJECT(ModelResource);
            //}
            END_SERIALIZE();
        }

        bool Deserialize(const json& j) 
        {
            ModelPath = j["ModelPath"];

            if (j.contains("ModelResource"))
            {
                ModelResource.ID = UUID(j["ModelResource"]);
            }

            //ModelResource = CreateRef<Model>();
            //
            //if (ModelPath.empty() || !String::EndsWith(ModelPath, ".mesh"))
            //{
            //    if (j.contains("ModelResource"))
            //    {
            //        auto& res = j["ModelResource"];
            //        ModelResource->Deserialize(res);
            //    }
            //}
            //else
            //{
            //    ModelResource = ResourceLoader::LoadModel(ModelPath);
            //}

            return true;
        }
    };
}

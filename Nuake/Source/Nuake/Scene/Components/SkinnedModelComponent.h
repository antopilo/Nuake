#pragma once

#include "Component.h"
#include "FieldTypes.h"

#include "Nuake/FileSystem/File.h"
#include "Nuake/FileSystem/FileSystem.h"
#include "Nuake/Resource/Serializable.h"
#include "Nuake/Resource/SkinnedModel.h"

#include <string>
#include <vector>

namespace Nuake
{
    class Scene;
    
    class SkinnedModelComponent : public Component
    {
        NUAKECOMPONENT(SkinnedModelComponent, "Skinned Model")

        static void InitializeComponentClass()
        {
            //BindComponentField<&SkinnedModelComponent::ModelPath>("ModelPath", "Model Path");
            BindComponentProperty<&SkinnedModelComponent::SetModel, &SkinnedModelComponent::GetModel>("ModelPath", "Model");
            ResourceFileRestriction("_Model");

            BindComponentProperty<&SkinnedModelComponent::SetPlaying, &SkinnedModelComponent::GetIsPlaying>("Playing", "Playing");
                SetFlags(ComponentFieldTrait::Transient);

            BindComponentProperty<&SkinnedModelComponent::SetAnimationList, &SkinnedModelComponent::GetAnimationList>("Animation", "Animation");
                SetFlags(ComponentFieldTrait::Transient);
        }

        Ref<SkinnedModel> ModelResource;
        ResourceFile ModelPath;
        DynamicItemList animationList;

        void SetModel(ResourceFile file);
        ResourceFile GetModel()
        {
            return ModelPath;
        }

        void LoadModel(entt::entity e, Scene* scene);

        void SetPlaying(bool play);
        bool GetIsPlaying() const;

        DynamicItemList& GetAnimationList() { return animationList; }
        void SetAnimationList(int i);

        void RegenerateAnimationList();

        json Serialize();

        bool Deserialize(const json& j);
    };
}

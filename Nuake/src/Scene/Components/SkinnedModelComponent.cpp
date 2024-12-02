#include "SkinnedModelComponent.h"
#include "src/Resource/ModelLoader.h"
#include "src/Scene/Entities/Entity.h"

namespace Nuake
{
	void SkinnedModelComponent::SetModel(ResourceFile file)
	{
		ModelPath = file;
	}

	void SkinnedModelComponent::LoadModel(entt::entity e, Scene* scene)
	{
		ModelLoader loader = ModelLoader();
		this->ModelResource = loader.LoadSkinnedModel(ModelPath.GetRelativePath());

		Entity entity = Entity{ e, scene };
		scene->CreateSkeleton(entity);

		RegenerateAnimationList();
	}

	void SkinnedModelComponent::SetPlaying(bool play)
	{
		if (ModelResource == nullptr)
			return;
		ModelResource->IsPlaying = play;
	}

	bool SkinnedModelComponent::GetIsPlaying() const
	{
		if (ModelResource == nullptr)
			return false;
		return ModelResource->IsPlaying;
	}

	void SkinnedModelComponent::SetAnimationList(int i)
	{
		animationList.index = i;

		// If the model is valid then play the animation
		if (ModelResource != nullptr)
		{
			ModelResource->PlayAnimation(animationList.index);
		}
	}

	void SkinnedModelComponent::RegenerateAnimationList()
	{
		if (ModelResource == nullptr)
		{
			return;
		}
		
		// Regenerate the animation list
		std::vector<Ref<SkeletalAnimation>> animations = ModelResource->GetAnimations();
		for (uint32_t i = 0; i < animations.size(); i++)
		{
			Ref<SkeletalAnimation>& animation = animations[i];
			if (animation == nullptr)
			{
				continue;
			}

			animationList.items.push_back(animation->GetName());
		}
	}

	json SkinnedModelComponent::Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_RES_FILE(ModelPath);

		if (ModelResource)
		{
			SERIALIZE_OBJECT(ModelResource);
		}
		
		j["CurrentAnimationIndex"] = animationList.index;

		END_SERIALIZE();
	}

	bool SkinnedModelComponent::Deserialize(const json& j)
	{
		DESERIALIZE_RES_FILE(ModelPath);
            
		ModelResource = CreateRef<SkinnedModel>();

		if (j.contains("ModelResource"))
		{
			auto& res = j["ModelResource"];
			ModelResource->Deserialize(res);
		}

		RegenerateAnimationList();

		if (j.contains("CurrentAnimationIndex"))
		{
			int animIndex = j["CurrentAnimationIndex"];
			if (animIndex > 0 && animIndex < animationList.items.size())
			{
				SetAnimationList(animIndex);
			}
		}

		return true;
	}
}

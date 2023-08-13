#include "SkinnedModelComponent.h"
#include "src/Resource/ModelLoader.h"

namespace Nuake
{
	SkinnedModelComponent::SkinnedModelComponent()
	{

	}

	void SkinnedModelComponent::LoadModel()
	{
		auto loader = ModelLoader();
		this->ModelResource = loader.LoadSkinnedModel(ModelPath);
	}
}
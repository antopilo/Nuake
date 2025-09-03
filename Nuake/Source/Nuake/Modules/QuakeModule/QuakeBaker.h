#pragma once

#include <Nuake/Resource/Bakers/IAssetBaker.h>

namespace Nuake
{

	class QuakeBaker : public IAssetBaker
	{
	private:
		std::string currentPath;

	public:
		QuakeBaker() : IAssetBaker({ ".bsp" }) {}

		Ref<File> Bake(const Ref<File>& file) override;
	};
}
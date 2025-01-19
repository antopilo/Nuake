#pragma once
#include "IAssetBaker.h"

#include <string>
#include <map>

namespace Nuake
{
	class AssetBakerManager
	{
	private:
		std::map<std::string, Ref<IAssetBaker>> Bakers;

	public:
		static AssetBakerManager& Get()
		{
			static AssetBakerManager instance;
			return instance;
		}

		void RegisterBaker(Ref<IAssetBaker> baker)
		{
			Bakers[baker->GetExtension()] = baker;
		}
	};
}
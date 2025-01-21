#pragma once
#include "IAssetBaker.h"
#include "src/Core/Core.h"
#include "src/FileSystem/File.h"

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
			// Latest registered baker overwrites previous
			for(const auto& extension : baker->GetExtensions())
			{
				Bakers[extension] = baker;
			}
		}
		
		Ref<File> Bake(const Ref<File>& file)
		{
			const std::string extension = file->GetExtension();
			if(Bakers.find(extension) == Bakers.end())
			{
				assert(false && "Baker not found for asset type");
				return nullptr;
			}
			
			return Bakers[extension]->Bake(file);
		}
	};
}
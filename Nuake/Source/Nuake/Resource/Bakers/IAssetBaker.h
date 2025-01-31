#pragma once

#include "Nuake/Core/Core.h"
#include "Nuake/FileSystem/File.h"

#include <set>
#include <span>
#include <string>

namespace Nuake
{
	class IAssetBaker
	{
	private:
		std::set<std::string> Extensions;
		
	public:
		IAssetBaker(const std::string& extension) : Extensions { extension } {}
		IAssetBaker(std::initializer_list<std::string> extension) : Extensions(extension.begin(), extension.end()) {}
		virtual ~IAssetBaker() = default;

		std::set<std::string> GetExtensions() const { return Extensions; }
		
		inline bool CanBakeFile(const Ref<File>& file) const
		{
			return Extensions.find(file->GetExtension()) != Extensions.end();
		}
		
	public:
		virtual Ref<File> Bake(const Ref<File>& file) = 0;
	};
}
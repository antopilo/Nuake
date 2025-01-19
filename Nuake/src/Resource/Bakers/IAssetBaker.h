#pragma once

#include "src/Core/Core.h"
#include "src/FileSystem/File.h"

#include <string>

namespace Nuake
{
	class IAssetBaker
	{
	private:
		std::string Extension;

	public:
		IAssetBaker(const std::string& extension) : Extension(extension) {}
		virtual ~IAssetBaker() = default;

		std::string GetExtension() const { return Extension; }

	public:
		virtual Ref<File> Bake(const Ref<File>& file) = 0;
	};
}
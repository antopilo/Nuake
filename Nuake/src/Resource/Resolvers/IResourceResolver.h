#pragma once

#include "src/Core/Core.h"
#include "src/FileSystem/File.h"
#include "src/Resource/UUID.h"
#include "src/Resource/Resource.h"

#include <map>
#include <string>

namespace Nuake
{
	class IResourceResolver
	{
	private:
		std::string Extension;
		
	public:
		IResourceResolver(const std::string& extension) : Extension(extension) {}
		virtual ~IResourceResolver() = default;

		// Converts a file to a resource UUID
		virtual UUID ResolveUUID(const Ref<File>& file) = 0;
		virtual Ref<Resource> Resolve(const Ref<File>& file) = 0;

		std::string GetExtension() const { return Extension; }
	};
}
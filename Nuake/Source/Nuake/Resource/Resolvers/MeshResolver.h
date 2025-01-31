#pragma once
#include "IResourceResolver.h"

#include "Nuake/Resource/Resource.h"

namespace Nuake
{
	class MeshResolver : public IResourceResolver
	{
	public:
		MeshResolver() : IResourceResolver(".nkmesh") {}
		UUID ResolveUUID(const Ref<File>& file) override;
		Ref<Resource> Resolve(const Ref<File>& file) override;
	};

	class MaterialResolver : public IResourceResolver
	{
	public:
		MaterialResolver() : IResourceResolver(".material") {}
		UUID ResolveUUID(const Ref<File>& file) override;
		Ref<Resource> Resolve(const Ref<File>& file) override;
	};
}
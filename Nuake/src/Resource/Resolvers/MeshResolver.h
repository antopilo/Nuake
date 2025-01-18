#pragma once
#include "IResourceResolver.h"

#include "src/Resource/Resource.h"

namespace Nuake
{
	class MeshResolver : public IResourceResolver
	{
	public:
		MeshResolver() : IResourceResolver(".nkmesh") {}
		Ref<Resource> Resolve(const Ref<File>& file) override;
	};

	class MaterialResolver : public IResourceResolver
	{
	public:
		MaterialResolver() : IResourceResolver(".material") {}
		Ref<Resource> Resolve(const Ref<File>& file) override;
	};
}
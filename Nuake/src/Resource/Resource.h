#pragma once
#include <string>
#include <vector>
#include "src/Rendering/Mesh/Mesh.h"

namespace Nuake {
	enum class ResourceType
	{
		Texture,
		Prefab,
		Map,
		Mesh,
		Shader
	};


	class Resource
	{
	public:
		unsigned int Id;
		ResourceType Type;

		bool IsEmbedded;
		std::string Path; // Only if embedded

		void MakeExternal();
		void Duplicate();
		void MakeEmbedded();
	};

	class TextureResource : Resource
	{
	public:
		std::string TexturePath;
		// texture flags
	};


	class ModelResource : Resource
	{
	public:
		std::vector<Mesh*> SubMeshes;

		ModelResource();
		bool Load(const std::string& path);
	};
}

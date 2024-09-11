#pragma once
#include "src/Core/Core.h"
#include "src/Resource/UUID.h"
#include "src/Resource/Serializable.h"

namespace Nuake
{
	class Material;
	class Model;
	class UIResource;

	class ResourceLoader
	{
	private:
		const std::string FILE_NOT_FOUND = "[Resource Loader] File doesn't exists. \n ";
		const std::string WRONG_EXTENSION = "[Resource Loader] Resource type mismatch file extension. \n expected: ";
		const std::string MATERIAL_EXT = ".material";

	public:
		ResourceLoader() = default;
		~ResourceLoader() = default;

		static Ref<Material> LoadMaterial(const std::string& path);
		static Ref<Model> LoadModel(const std::string& path);
		static Ref<UIResource> LoadUI(const std::string& path);

	private:
		static UUID ReadUUID(json j);
	};
}
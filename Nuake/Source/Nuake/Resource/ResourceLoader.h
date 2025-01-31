#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Resource/UUID.h"
#include "Nuake/Resource/Serializable.h"

namespace Nuake
{
	class Material;
	class Model;
	class UIResource;
	class File;
	class SkyResource;
	class Environment;

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
		static Ref<SkyResource> LoadSky(const std::string& path);
		static Ref<Environment> LoadEnvironment(const std::string& path);
		static Ref<UIResource> LoadUI(const std::string& path);
		static Ref<UIResource> LoadUI(const Ref<File>& file);

	private:
		static UUID ReadUUID(json j);
	};
}
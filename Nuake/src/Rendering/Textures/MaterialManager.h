#pragma once
#include <map>
#include <string>
#include "src/Core/Core.h"

namespace Nuake
{
	class Material;

	// TODO: Should probably be static.
	class MaterialManager 
	{
	private:
		static Ref<MaterialManager> s_Instance;

		std::map<std::string, Ref<Material>> m_Materials;

		// TODO: Pile of crap
		Material* ParseMaterialFile(const std::string path);
		void SaveMaterialFile(const std::string path, Material* material);
		bool IsMaterialLoaded(const std::string path);


	public:
		const std::string DEFAULT_MATERIAL = "resources/Textures/default/Default.png";
		std::string CurrentlyBoundedMaterial = "";
		MaterialManager();

		void LoadMaterials();

		void RegisterMaterial(Ref<Material> material);

		Ref<Material> LoadMaterial(std::string path);

		Ref<Material> GetMaterial(const std::string name);

		std::map<std::string, Ref<Material>> GetAllMaterials();

		static Ref<MaterialManager> Get()
		{
			if (!s_Instance)
				s_Instance = CreateRef<MaterialManager>();

			return s_Instance;
		}
	};
}

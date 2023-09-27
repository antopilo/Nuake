#pragma once
#include "src/Core/Core.h"
#include "src/Resource/UUID.h"

#include <map>
#include <string>


namespace Nuake
{
	class Material;

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
		const std::string DEFAULT_MATERIAL = "Resources/Textures/Default.png";
		UUID CurrentlyBoundedMaterial;
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

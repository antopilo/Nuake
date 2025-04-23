#pragma once
#include "Nuake/Core/Core.h"

namespace Nuake
{
	class Texture;
	class VulkanImage;

	// Todo: SHOULD probably be static too.
	class TextureManager
	{
	private:
		static TextureManager* s_Instance;

		static std::map<std::string, Ref<Texture>> m_Registry;
		static std::map<std::string, Ref<VulkanImage>> m_Registry2;
		bool IsTextureLoaded(const std::string path);

		void LoadStaticTextures();

	public:
		static TextureManager* Get();

		TextureManager();
		bool IsTextureLoaded2(const std::string path);

		Ref<Texture> GetTexture(const std::string path);
		Ref<VulkanImage> GetTexture2(const std::string path);
		
	};
}

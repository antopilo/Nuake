#pragma once
#include "src/Core/Core.h"

namespace Nuake
{
	class Texture;

	// Todo: SHOULD probably be static too.
	class TextureManager
	{
	private:
		static TextureManager* s_Instance;

		static std::map<std::string, Ref<Texture>> m_Registry;
		bool IsTextureLoaded(const std::string path);

		void LoadStaticTextures();

	public:
		static TextureManager* Get();

		TextureManager();

		Ref<Texture> GetTexture(const std::string path);

		
	};
}

#pragma once
#include <map>
#include <string>
#include "../Core/Core.h"
class Texture;

// Todo: SHOULD probably be static too.
class TextureManager
{
private:
	static TextureManager* s_Instance;

	static std::map<std::string, Ref<Texture>> m_Registry;
	bool IsTextureLoaded(const std::string path);

public:
	static TextureManager* Get();

	TextureManager(); 

	Ref<Texture> GetTexture(const std::string path);
};
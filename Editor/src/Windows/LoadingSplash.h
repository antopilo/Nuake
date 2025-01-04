#pragma once
#include <src/Core/Core.h>
#include <src/Rendering/Textures/Texture.h>

#include <string>

namespace Nuake
{
	class Texture;
	class VulkanImage;
}

class LoadingSplash
{
private:
	const std::string NUAKE_LOGO_PATH = "Resources/Images/logo_white.png";
	Ref<Nuake::VulkanImage> _NuakeLogo;

public:
	static LoadingSplash& Get()
	{
		static LoadingSplash instance;
		return instance;
	}

	LoadingSplash();
	~LoadingSplash() = default;

	void Draw();
};
#pragma once
#include <Nuake/Core/Core.h>
#include <Nuake/Rendering/Textures/Texture.h>

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
	const std::string NUAKE_SPLASH_PATH = "Resources/Images/splash.png";
	Ref<Nuake::VulkanImage> _NuakeLogo;
	Ref<Nuake::VulkanImage> _NuakeSplash;
public:
	static LoadingSplash& Get()
	{
		static LoadingSplash instance;
		return instance;
	}

	LoadingSplash();
	~LoadingSplash() = default;

	void Draw(const std::string& projectPath);
};
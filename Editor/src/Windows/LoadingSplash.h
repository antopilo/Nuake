#pragma once
#include <src/Core/Core.h>
#include <src/Rendering/Textures/Texture.h>

#include <string>

namespace Nuake
{
	class Texture;
}

class LoadingSplash
{
private:
	const std::string NUAKE_LOGO_PATH = "resources/Images/logo_white.png";
	Ref<Nuake::Texture> _NuakeLogo;

public:
	static LoadingSplash& LoadingSplash::Get()
	{
		static LoadingSplash instance;
		return instance;
	}

	LoadingSplash();
	~LoadingSplash() = default;

	void Draw();
};
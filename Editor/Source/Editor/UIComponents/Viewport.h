#pragma once
#include "Engine.h"
#include "Nuake/Rendering/Textures/Texture.h"

class Viewport
{
public:
	std::string Name = "";
	Ref<Nuake::Texture> Texture;

	Viewport(const std::string& name, Ref<Nuake::Texture> texture);
	void Draw();
};
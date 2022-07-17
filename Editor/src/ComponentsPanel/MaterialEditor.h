#pragma once
#include <src/Core/Core.h>
#include <src/Rendering/Textures/Material.h>

class MaterialEditor
{
public:
	MaterialEditor() = default;
	~MaterialEditor() = default;

	void Draw(Ref<Nuake::Material> material);
};
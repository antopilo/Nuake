#pragma once
#include <Nuake/Core/Core.h>
#include <Nuake/Rendering/Textures/Material.h>

class MaterialEditor
{
public:
	MaterialEditor() = default;
	~MaterialEditor() = default;

	void Draw(Ref<Nuake::Material> material);
};
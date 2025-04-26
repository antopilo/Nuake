#pragma once
#include <Nuake/Core/Core.h>
#include <Nuake/Rendering/Textures/Material.h>
#include <Nuake/Scene/Scene.h>

#include "Nuake/Rendering/Vulkan/SceneViewport.h"
class MaterialEditor
{
private:
	Ref<Nuake::Scene> PreviewScene;
	Ref<Nuake::Viewport> SceneViewport;
public:
	MaterialEditor();
	~MaterialEditor() = default;

	void Draw(Ref<Nuake::Material> material);
};
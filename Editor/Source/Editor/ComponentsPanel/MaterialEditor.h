#pragma once
#include <Nuake/Core/Core.h>
#include <Nuake/Rendering/Textures/Material.h>
#include <Nuake/Scene/Scene.h>

#include "Nuake/Rendering/Vulkan/SceneViewport.h"

class MaterialEditor
{
private:
	static Ref<Nuake::Scene> PreviewScene;
	static Ref<Nuake::Viewport> SceneViewport;

public:
	MaterialEditor();
	~MaterialEditor() = default;

	void Disable();
	void Enable();
	void Draw(Ref<Nuake::Material> material);
};
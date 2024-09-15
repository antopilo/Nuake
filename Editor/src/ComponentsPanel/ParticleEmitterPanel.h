#pragma once
#include "ComponentPanel.h"

#include "src/FileSystem/FileSystem.h"
#include <src/Core/Maths.h>
#include <src/Scene/Components/ParticleEmitterComponent.h>
#include <src/Scene/Entities/ImGuiHelper.h>
#include "MaterialEditor.h"
#include <src/Resource/ResourceLoader.h>
#include <src/Resource/ResourceLoader.h>

class ParticleEmitterPanel : ComponentPanel
{
public:
	Scope<ModelResourceInspector> _modelInspector;

	ParticleEmitterPanel() {}

	void Draw(Nuake::Entity entity) override
	{
		
	}
};
#pragma once
#include "ComponentPanel.h"

#include "src/FileSystem/FileSystem.h"
#include <src/Core/Maths.h>
#include <src/Scene/Components/SpriteComponent.h>
#include <src/Scene/Entities/ImGuiHelper.h>


class SpritePanel : ComponentPanel
{
public:
	SpritePanel() = default;
	~SpritePanel() = default;

	void Draw(Nuake::Entity entity) override
	{
		
	}
};
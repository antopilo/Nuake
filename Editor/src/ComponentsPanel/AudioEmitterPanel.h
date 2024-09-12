#pragma once
#include "ComponentPanel.h"

#include "src/FileSystem/FileSystem.h"
#include <src/Core/Maths.h>
#include <src/Scene/Components/AudioEmitterComponent.h>
#include <src/Scene/Entities/ImGuiHelper.h>


class AudioEmitterPanel : ComponentPanel
{
public:
    AudioEmitterPanel() = default;
    ~AudioEmitterPanel() = default;

	void Draw(Nuake::Entity entity) override
	{
		
	}
};
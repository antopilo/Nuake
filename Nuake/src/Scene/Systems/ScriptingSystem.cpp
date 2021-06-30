#include "ScriptingSystem.h"
#include <src/Scene/Components/WrenScriptComponent.h>
#include "src/Scene/Scene.h"

ScriptingSystem::ScriptingSystem(Scene* scene)
{
	m_Scene = scene;
}

void ScriptingSystem::Init()
{
	ScriptingEngine::Init();

	auto entities = m_Scene->m_Registry.view<WrenScriptComponent>();
	for (auto e : entities)
	{
		WrenScriptComponent& wren = entities.get<WrenScriptComponent>(e);
		if (wren.Script != "" && wren.Class != "")
			wren.WrenScript = CreateRef<WrenScript>(wren.Script, wren.Class, true);

		if (wren.WrenScript != nullptr)
		{
			wren.WrenScript->SetScriptableEntityID((int)e);
			wren.WrenScript->CallInit();
		}
	}
}


void ScriptingSystem::Update(Timestep ts)
{
	auto entities = m_Scene->m_Registry.view<WrenScriptComponent>();
	for (auto& e : entities)
	{
		WrenScriptComponent& wren = entities.get<WrenScriptComponent>(e);

		if (wren.WrenScript != nullptr)
			wren.WrenScript->CallUpdate(ts);
	}
}


void ScriptingSystem::FixedUpdate(Timestep ts)
{
	auto entities = m_Scene->m_Registry.view<WrenScriptComponent>();
	for (auto& e : entities)
	{
		WrenScriptComponent& wren = entities.get<WrenScriptComponent>(e);

		if (wren.WrenScript != nullptr)
			wren.WrenScript->CallFixedUpdate(ts);
	}
}


void ScriptingSystem::Exit()
{
	auto entities = m_Scene->m_Registry.view<WrenScriptComponent>();
	for (auto& e : entities)
	{
		WrenScriptComponent& wren = entities.get<WrenScriptComponent>(e);

		if (wren.WrenScript != nullptr)
			wren.WrenScript->CallExit();
	}

	ScriptingEngine::Close();
}
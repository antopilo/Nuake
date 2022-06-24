#include "ScriptingSystem.h"
#include "src/Scene/Components/WrenScriptComponent.h"
#include "src/Scene/Scene.h"
#include "Engine.h"

namespace Nuake {
	ScriptingSystem::ScriptingSystem(Scene* scene)
	{
		m_Scene = scene;
	}

	bool ScriptingSystem::Init()
	{
		ScriptingEngine::Init();

		auto& entities = m_Scene->m_Registry.view<WrenScriptComponent>();
		for (auto& e : entities)
		{
			WrenScriptComponent& wren = entities.get<WrenScriptComponent>(e);

			if (!wren.mWrenScript)
				continue;

			wren.mWrenScript->Build(wren.mModule, true);
			
			if (!wren.mWrenScript->HasCompiledSuccesfully())
				return false;

			wren.mWrenScript->SetScriptableEntityID((int)e);
			wren.mWrenScript->CallInit();
		}

		return true;
	}

	void ScriptingSystem::Update(Timestep ts)
	{
		if (!Engine::IsPlayMode)
			return;

		auto entities = m_Scene->m_Registry.view<WrenScriptComponent>();
		for (auto& e : entities)
		{
			WrenScriptComponent& wren = entities.get<WrenScriptComponent>(e);

			if (wren.mWrenScript != nullptr)
				wren.mWrenScript->CallUpdate(ts);
		}
	}


	void ScriptingSystem::FixedUpdate(Timestep ts)
	{
		auto entities = m_Scene->m_Registry.view<WrenScriptComponent>();
		for (auto& e : entities)
		{
			WrenScriptComponent& wren = entities.get<WrenScriptComponent>(e);

			if (wren.mWrenScript != nullptr)
				wren.mWrenScript->CallFixedUpdate(ts);
		}
	}


	void ScriptingSystem::Exit()
	{
		auto entities = m_Scene->m_Registry.view<WrenScriptComponent>();
		for (auto& e : entities)
		{
			WrenScriptComponent& wren = entities.get<WrenScriptComponent>(e);

			if (wren.mWrenScript != nullptr)
				wren.mWrenScript->CallExit();
		}

		ScriptingEngine::Close();
	}
}

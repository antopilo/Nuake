#include "ScriptingSystem.h"
#include "src/Scene/Components/WrenScriptComponent.h"
#include "src/Scene/Scene.h"

namespace Nuake {
	ScriptingSystem::ScriptingSystem(Scene* scene)
	{
		m_Scene = scene;
	}

	bool ScriptingSystem::Init()
	{
		ScriptingEngine::Init();

		auto entities = m_Scene->m_Registry.view<WrenScriptComponent>();
		for (auto e : entities)
		{
			WrenScriptComponent& wren = entities.get<WrenScriptComponent>(e);
			if (wren.Script != "" && wren.Class != "")
			{
				wren.mWrenScript = CreateRef<WrenScript>(wren.Script, wren.Class, true);
				if (!wren.mWrenScript->CompiledSuccesfully)
					return false;
			}

			if (wren.mWrenScript != nullptr)
			{
				wren.mWrenScript->SetScriptableEntityID((int)e);
				wren.mWrenScript->CallInit();
			}
		}

		return true;
	}


	void ScriptingSystem::Update(Timestep ts)
	{
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

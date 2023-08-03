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

		Logger::Log("Initializing ScriptingSystem");

		auto& entities = m_Scene->m_Registry.view<WrenScriptComponent>();
		for (auto& e : entities)
		{
			WrenScriptComponent& wren = entities.get<WrenScriptComponent>(e);

			if (!wren.mWrenScript)
				continue;

			wren.mWrenScript->Build(wren.mModule, true);
			
			if (!wren.mWrenScript->HasCompiledSuccesfully())
			{
				Logger::Log("ScriptingSystem failed");
				return false;
			}

			wren.mWrenScript->SetScriptableEntityID((int)e);
			wren.mWrenScript->CallInit();
		}

		return true;
	}

	void ScriptingSystem::Update(Timestep ts)
	{
		if (!Engine::IsPlayMode())
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
		if (!Engine::IsPlayMode())
			return;

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

			try
			{
				if (wren.mWrenScript != nullptr)
					wren.mWrenScript->CallExit();
			}
			catch (std::exception* e)
			{
			}
			
		}

		ScriptingEngine::Close();
	}
}

#include "ScriptingSystem.h"
#include "src/Scene/Components/WrenScriptComponent.h"
#include "src/Scene/Components/NetScriptComponent.h"
#include "src/Scene/Scene.h"
#include "Engine.h"

#include "src/Scripting/ScriptingEngineNet.h"


namespace Nuake 
{
	ScriptingSystem::ScriptingSystem(Scene* scene)
	{
		m_Scene = scene;
	}

	bool ScriptingSystem::Init()
	{
		ScriptingEngine::Init();

		Logger::Log("Initializing ScriptingSystem");

		auto wrenEntities = m_Scene->m_Registry.view<WrenScriptComponent>();
		for (auto& e : wrenEntities)
		{
			WrenScriptComponent& wren = wrenEntities.get<WrenScriptComponent>(e);

			if (!wren.mWrenScript)
				continue;

			wren.mWrenScript->Build(wren.mModule, true);
			
			if (!wren.mWrenScript->HasCompiledSuccesfully())
			{
				Logger::Log("Failed to compile Wren script: " + wren.Script, "scripting system");
				return false;
			}

			wren.mWrenScript->SetScriptableEntityID((int)e);
			wren.mWrenScript->CallInit();
		}

		auto& scriptingEngineNet = ScriptingEngineNet::Get();
		scriptingEngineNet.Initialize();
		scriptingEngineNet.LoadProjectAssembly(Engine::GetProject());

		auto netEntities = m_Scene->m_Registry.view<NetScriptComponent>();
		for (auto& e : netEntities)
		{
			NetScriptComponent& netScriptComponent = netEntities.get<NetScriptComponent>(e);

			if (netScriptComponent.ScriptPath.empty())
				continue;

			// Creates an instance of the entity script in C#
			auto entity = Entity{ e, m_Scene };
			scriptingEngineNet.RegisterEntityScript(entity);

			// We can now call on init on it.
			auto scriptInstance = scriptingEngineNet.GetEntityScript(entity);
			scriptInstance.InvokeMethod("OnInit");
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

		auto& scriptingEngineNet = ScriptingEngineNet::Get();
		auto netEntities = m_Scene->m_Registry.view<NetScriptComponent>();
		for (auto& e : netEntities)
		{
			NetScriptComponent& netScriptComponent = netEntities.get<NetScriptComponent>(e);

			if (netScriptComponent.ScriptPath.empty())
				continue;

			auto entity = Entity{ e, m_Scene };
			auto scriptInstance = scriptingEngineNet.GetEntityScript(entity);
			scriptInstance.InvokeMethod("OnUpdate", ts.GetSeconds());
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

		auto& scriptingEngineNet = ScriptingEngineNet::Get();
		auto netEntities = m_Scene->m_Registry.view<NetScriptComponent>();
		for (auto& e : netEntities)
		{
			NetScriptComponent& netScriptComponent = netEntities.get<NetScriptComponent>(e);

			if (netScriptComponent.ScriptPath.empty())
				continue;

			auto entity = Entity{ e, m_Scene };
			auto scriptInstance = scriptingEngineNet.GetEntityScript(entity);
			scriptInstance.InvokeMethod("OnFixedUpdate", ts.GetSeconds());
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

		auto& scriptingEngineNet = ScriptingEngineNet::Get();
		auto netEntities = m_Scene->m_Registry.view<NetScriptComponent>();
		for (auto& e : netEntities)
		{
			NetScriptComponent& netScriptComponent = netEntities.get<NetScriptComponent>(e);

			if (netScriptComponent.ScriptPath.empty())
				continue;

			// Creates an instance of the entity script in C#
			auto entity = Entity{ e, m_Scene };
			auto scriptInstance = scriptingEngineNet.GetEntityScript(entity);
			scriptInstance.InvokeMethod("OnDestroy");
		}

		ScriptingEngine::Close();
		ScriptingEngineNet::Get().Uninitialize();
	}
}

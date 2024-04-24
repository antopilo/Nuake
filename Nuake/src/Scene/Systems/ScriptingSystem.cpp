#include "ScriptingSystem.h"
#include "src/Scene/Components/WrenScriptComponent.h"
#include "src/Scene/Components/NetScriptComponent.h"
#include "src/Scene/Scene.h"
#include "Engine.h"

#include "src/Scripting/ScriptingEngineNet.h"
#include "src/Physics/PhysicsManager.h"

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

			auto entity = Entity{ e, m_Scene };

			// Creates an instance of the entity script in C#
			scriptingEngineNet.RegisterEntityScript(entity);

			netScriptComponent.Initialized = true;
		}

		for (auto& e : netEntities)
		{
			auto entity = Entity{ e, m_Scene };

			if (entity.IsValid() && scriptingEngineNet.HasEntityScriptInstance(entity))
			{
				// We can now call on init on it.
				auto scriptInstance = scriptingEngineNet.GetEntityScript(entity);
				scriptInstance.InvokeMethod("OnInit");
			}
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

		// Instance all the new entities
		std::vector<Entity> entityJustInstanced;
		for (auto& e : netEntities)
		{
			auto& netScriptComponent = netEntities.get<NetScriptComponent>(e);
			if (!netScriptComponent.Initialized)
			{
				if (netScriptComponent.ScriptPath.empty())
					continue;

				auto entity = Entity{ e, m_Scene };

				// Creates an instance of the entity script in C#
				scriptingEngineNet.RegisterEntityScript(entity);

				netScriptComponent.Initialized = true;
				entityJustInstanced.push_back(entity);
			}
		}

		// Call init on the newly created instance
		for (auto& e : entityJustInstanced)
		{
			auto& netScriptComponent = e.GetComponent<NetScriptComponent>();
			if (e.IsValid() && scriptingEngineNet.HasEntityScriptInstance(e))
			{
				// We can now call on init on it.
				auto scriptInstance = scriptingEngineNet.GetEntityScript(e);
				scriptInstance.InvokeMethod("OnInit");
			}
		}

		// Then call update on all the other scripts
		for (auto& e : netEntities)
		{
			NetScriptComponent& netScriptComponent = netEntities.get<NetScriptComponent>(e);

			if (netScriptComponent.ScriptPath.empty())
				continue;

			auto entity = Entity{ e, m_Scene };
			auto scriptInstance = scriptingEngineNet.GetEntityScript(entity);
			scriptInstance.InvokeMethod("OnUpdate", ts.GetSeconds());
		}

		DispatchPhysicCallbacks();
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

	void ScriptingSystem::DispatchPhysicCallbacks()
	{
		auto& scriptingEngineNet = ScriptingEngineNet::Get();

		auto& physicsManager = PhysicsManager::Get();
		const auto collisions = physicsManager.GetCollisions();
		for (const auto& col : collisions)
		{
			// Calling both collidee(?)
			Entity entity1 = { (entt::entity)col.Entity1, m_Scene };
			if (entity1.IsValid() && scriptingEngineNet.HasEntityScriptInstance(entity1))
			{
				auto scriptInstance = scriptingEngineNet.GetEntityScript(entity1);
				scriptInstance.InvokeMethod("OnCollisionInternal", (int)col.Entity2);
			}

			Entity entity2 = { (entt::entity)col.Entity2, m_Scene };
			if (entity2.IsValid() && scriptingEngineNet.HasEntityScriptInstance(entity2))
			{
				auto scriptInstance = scriptingEngineNet.GetEntityScript(entity2);
				scriptInstance.InvokeMethod("OnCollisionInternal", (int)col.Entity1);
			}
		}

		physicsManager.GetWorld()->ClearCollisionData();
	}
}

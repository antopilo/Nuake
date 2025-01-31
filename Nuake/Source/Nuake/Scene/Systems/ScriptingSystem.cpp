#include "ScriptingSystem.h"
#include "Nuake/Scene/Components/NetScriptComponent.h"
#include "Nuake/Scene/Scene.h"
#include "Engine.h"

#include "Nuake/Scripting/ScriptingEngineNet.h"
#include "Nuake/Physics/PhysicsManager.h"
#include "Nuake/UI/Parsers/CanvasParser.h"

namespace Nuake 
{
	ScriptingSystem::ScriptingSystem(Scene* scene)
	{
		m_Scene = scene;
	}

	bool ScriptingSystem::Init()
	{
		Logger::Log("Initializing ScriptingSystem");

		preInitDelegate.Broadcast();

		auto& scriptingEngineNet = ScriptingEngineNet::Get();
		scriptingEngineNet.Uninitialize();
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

		// Instantiate UI widgets
		for (auto& uiWidget : CanvasParser::Get().GetAllCustomWidgetInstance())
		{
			scriptingEngineNet.RegisterCustomWidgetInstance(uiWidget.first.first, uiWidget.first.second, uiWidget.second);
		}

		// Call OnInit on entity script instances
		for (auto& e : netEntities)
		{
			auto entity = Entity{ e, m_Scene };

			if (entity.IsValid() && scriptingEngineNet.HasEntityScriptInstance(entity))
			{
				// We can now call on init on it.
				auto scriptInstance = scriptingEngineNet.GetEntityScript(entity);
				scriptInstance->InvokeMethod("OnInit");
			}
		}
		
		// Call OnInit on UI widgets
		for (auto& widget : CanvasParser::Get().GetAllCustomWidgetInstance())
		{
			const UUID& canvasInstanceUUID = widget.first.first;
			const UUID& widgetInstanceUUID = widget.first.second;
			if (scriptingEngineNet.HasCustomWidgetInstance(canvasInstanceUUID, widgetInstanceUUID))
			{
				auto widgetInstance = scriptingEngineNet.GetCustomWidgetInstance(canvasInstanceUUID, widgetInstanceUUID);
				widgetInstance.InvokeMethod("OnInit");
			}
		}

		postInitDelegate.Broadcast();

		return true;
	}

	void ScriptingSystem::Update(Timestep ts)
	{
		if (!Engine::IsPlayMode())
			return;

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
				scriptInstance->InvokeMethod("OnInit");
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
			scriptInstance->InvokeMethod("OnUpdate", ts.GetSeconds());
		}

		for (auto& widget : CanvasParser::Get().GetAllCustomWidgetInstance())
		{
			const UUID& canvasInstanceUUID = widget.first.first;
			const UUID& widgetInstanceUUID = widget.first.second;
			if (scriptingEngineNet.HasCustomWidgetInstance(canvasInstanceUUID, widgetInstanceUUID))
			{
				auto widgetInstance = scriptingEngineNet.GetCustomWidgetInstance(canvasInstanceUUID, widgetInstanceUUID);
				widgetInstance.InvokeMethod("OnTick", ts.GetSeconds());
			}
		}

		DispatchPhysicCallbacks();
	}

	void ScriptingSystem::FixedUpdate(Timestep ts)
	{
		if (!Engine::IsPlayMode())
			return;

		auto& scriptingEngineNet = ScriptingEngineNet::Get();
		auto netEntities = m_Scene->m_Registry.view<NetScriptComponent>();
		for (auto& e : netEntities)
		{
			NetScriptComponent& netScriptComponent = netEntities.get<NetScriptComponent>(e);

			if (netScriptComponent.ScriptPath.empty())
				continue;

			auto entity = Entity{ e, m_Scene };
			auto scriptInstance = scriptingEngineNet.GetEntityScript(entity);
			scriptInstance->InvokeMethod("OnFixedUpdate", ts.GetSeconds());
		}
	}

	void ScriptingSystem::Exit()
	{
		auto& scriptingEngineNet = ScriptingEngineNet::Get();
		auto netEntities = m_Scene->m_Registry.view<NetScriptComponent>();
		for (auto& e : netEntities)
		{
			NetScriptComponent& netScriptComponent = netEntities.get<NetScriptComponent>(e);

			if (netScriptComponent.ScriptPath.empty())
				continue;

			auto entity = Entity{ e, m_Scene };
			if (entity.IsValid() && scriptingEngineNet.HasEntityScriptInstance(entity))
			{
				Logger::Log(entity.GetComponent<NameComponent>().Name);
				auto scriptInstance = scriptingEngineNet.GetEntityScript(entity);
				scriptInstance->InvokeMethod("OnDestroy");
			}
		}

		ScriptingEngineNet::Get().Uninitialize();
	}

	void ScriptingSystem::InitializeNewScripts()
	{
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
				scriptInstance->InvokeMethod("OnCollisionInternal", (int)col.Entity2);
			}

			Entity entity2 = { (entt::entity)col.Entity2, m_Scene };
			if (entity2.IsValid() && scriptingEngineNet.HasEntityScriptInstance(entity2))
			{
				auto scriptInstance = scriptingEngineNet.GetEntityScript(entity2);
				scriptInstance->InvokeMethod("OnCollisionInternal", (int)col.Entity1);
			}
		}

		physicsManager.GetWorld()->ClearCollisionData();
	}
}

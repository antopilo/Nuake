#pragma once
#include "src/Scene/Systems/ScriptingSystem.h"
#include "src/Scene/Systems/PhysicsSystem.h"
#include "src/Scene/Systems/TransformSystem.h"
#include "src/Scene/Systems/QuakeMapBuilder.h"

#include "src/Rendering/SceneRenderer.h"
#include "Scene.h"
#include "Entities/Entity.h"

#include "src/Rendering/Renderer.h"
#include "src/Rendering/Textures/MaterialManager.h"
#include "src/Core/Physics/PhysicsManager.h"
#include "src/Core/Core.h"

#include <GL/glew.h>

#include "Engine.h"
#include "src/Core/FileSystem.h"
#include "src/Scene/Components/Components.h"
#include "src/Scene/Components/BoxCollider.h"
#include "src/Scene/Components/WrenScriptComponent.h"
#include "src/Scene/Components/BSPBrushComponent.h"
#include "src/Scene/Components/InterfaceComponent.h"

#include <fstream>
#include <streambuf>
#include <chrono>
#include "src/Core/OS.h"

namespace Nuake {
	Ref<Scene> Scene::New()
	{
		return CreateRef<Scene>();
	}

	Scene::Scene()
	{
		m_Systems = std::vector<Ref<System>>();
		m_EditorCamera = CreateRef<EditorCamera>();
		m_Environement = CreateRef<Environment>();

		// Adding systems - Order is important
		m_Systems.push_back(CreateRef<ScriptingSystem>(this));
		m_Systems.push_back(CreateRef<TransformSystem>(this));
		m_Systems.push_back(CreateRef<PhysicsSystem>(this));

		mSceneRenderer = new SceneRenderer();
		mSceneRenderer->Init();
	}

	Scene::~Scene() 
	{
	}

	std::string Scene::GetName()
	{
		return this->Name;
	}

	bool Scene::SetName(std::string& newName)
	{
		if (newName == "")
			return false;

		this->Name = newName;
		return true;
	}

	Entity Scene::GetEntity(int handle)
	{
		return Entity((entt::entity)handle, this);
	}

	Entity Scene::GetEntityByID(int id)
	{
		auto idView = m_Registry.view<NameComponent>();
		for (auto e : idView) 
		{
			NameComponent& nameC = idView.get<NameComponent>(e);
			if (nameC.ID == id)
			{
				return Entity{ e, this };
			}
		}

		assert("Not found");
	}

	bool Scene::OnInit()
	{
		for (auto& system : m_Systems)
		{
			if (!system->Init())
			{
				return false;
			}
		}

		return true;
	}

	void Scene::OnExit()
	{
		for (auto& system : m_Systems)
		{
			system->Exit();
		}
	}

	void Scene::Update(Timestep ts)
	{
		for (auto& system : m_Systems)
		{
			system->Update(ts);
		}
	}

	void Scene::FixedUpdate(Timestep ts)
	{
		for (auto& system : m_Systems)
		{
			system->FixedUpdate(ts);
		}
	}

	void Scene::EditorUpdate(Timestep ts)
	{
		
	}

	void Scene::Draw(FrameBuffer& framebuffer)
	{
		Ref<Camera> cam = nullptr;
		Matrix4 camTransform = Matrix4();
		{
			auto view = m_Registry.view<TransformComponent, CameraComponent, ParentComponent>();
			for (auto e : view) 
			{
				auto [transform, camera, parent] = view.get<TransformComponent, CameraComponent, ParentComponent>(e);
				cam = camera.CameraInstance;
				cam->Translation = transform.GetGlobalPosition();
				cam->SetDirection(Vector3(Vector4(0, 0, -1, 0) * transform.GetGlobalTransform()));
				camTransform = transform.GetGlobalTransform();

				break;
			}
		}

		if (!cam)
		{
			return;
		}

		mSceneRenderer->BeginRenderScene(cam->GetPerspective(), cam->GetTransform(), cam->Translation);
		mSceneRenderer->RenderScene(*this, framebuffer);
	}

	void Scene::Draw(FrameBuffer& framebuffer, const Matrix4& projection, const Matrix4& view)
	{
		mSceneRenderer->BeginRenderScene(m_EditorCamera->GetPerspective(), m_EditorCamera->GetTransform(), m_EditorCamera->Translation);
		mSceneRenderer->RenderScene(*this, framebuffer);
	}
	
	std::vector<Entity> Scene::GetAllEntities() 
	{
		std::vector<Entity> allEntities;
		auto view = m_Registry.view<NameComponent>();
		for (auto e : view) 
		{
			Entity newEntity(e, this);

			// Check if valid for deleted entities.
			if (newEntity.IsValid())
				allEntities.push_back(newEntity);
		}
		return allEntities;
	}

	Entity Scene::GetEntity(const std::string& name)
	{
		std::vector<Entity> allEntities;
		auto view = m_Registry.view<TransformComponent, NameComponent>();
		for (auto e : view) 
		{
			auto [transform, namec] = view.get<TransformComponent, NameComponent>(e);
			if (namec.Name == name)
				return Entity{ e, this };
		}

		return Entity();
	}

	Entity Scene::CreateEntity(const std::string& name) 
	{
		return CreateEntity(name, (int)OS::GetTime());
	}

	Entity Scene::CreateEntity(const std::string& name, int id)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<ParentComponent>();
		entity.AddComponent<VisibilityComponent>();

		NameComponent& nameComponent = entity.AddComponent<NameComponent>();
		nameComponent.Name = name;
		nameComponent.ID = id;

		Logger::Log("Created entity: " + nameComponent.Name, LOG_TYPE::VERBOSE);
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		ParentComponent& parentC = entity.GetComponent<ParentComponent>();
		std::vector<Entity> copyChildrens = parentC.Children;

		if (parentC.HasParent) 
		{  
			// Remove self from parents children lists.
			ParentComponent& parent = parentC.Parent.GetComponent<ParentComponent>();
			parent.RemoveChildren(entity);
		}

		for (auto& c : copyChildrens) 
		{
			Logger::Log("Deleting entity " + std::to_string(c.GetHandle()));
			DestroyEntity(c);
		}

		Logger::Log("Deleted entity" + std::to_string(entity.GetHandle()) + " - " + entity.GetComponent<NameComponent>().Name);
		entity.Destroy();
		m_Registry.shrink_to_fit();
	}

	Ref<Camera> Scene::GetCurrentCamera()
	{
		if (Engine::IsPlayMode)
		{
			Ref<Camera> cam = nullptr;
			{
				auto view = m_Registry.view<TransformComponent, CameraComponent>();
				for (auto e : view) 
				{
					auto [transform, camera] = view.get<TransformComponent, CameraComponent>(e);
					cam = camera.CameraInstance;
					break;
				}
			}

			if (!cam)
				cam = m_EditorCamera;
			return cam;
		}

		return m_EditorCamera;
	}

	Ref<Environment> Scene::GetEnvironment() const
	{
		return m_Environement;
	}

	void Scene::SetEnvironment(Ref<Environment> env)
	{
		m_Environement = env;
	}

	bool Scene::Save()
	{
		if (Path == "")
			Path = FileSystem::AbsoluteToRelative(FileDialog::SaveFile("*.scene") + ".scene");

		return SaveAs(Path);
	}

	bool Scene::SaveAs(const std::string& path)
	{
		std::string fileContent = Serialize().dump(4);

		FileSystem::BeginWriteFile(FileSystem::Root + path);
		FileSystem::WriteLine(fileContent);
		FileSystem::EndWriteFile();

		Logger::Log("Scene saved successfully");
		return true;
	}

	template<typename Component>
	void Scene::CopyComponent(entt::registry& dst, entt::registry& src)
	{
		auto view = src.view<Component>();
		for (auto e : view)
		{
			int id = src.get<NameComponent>(e).ID;
			auto& component = src.get<Component>(e);

			auto idView = dst.view<NameComponent>();
			for (auto de : idView)
			{
				if (idView.get<NameComponent>(de).ID == src.get<NameComponent>(e).ID)
				{
					dst.emplace_or_replace<Component>(de, component);
				}
			}
		}
	}

	Ref<Scene> Scene::Copy()
	{
		Ref<Scene> sceneCopy = CreateRef<Scene>();
		
		json serializedScene = Serialize();

		sceneCopy->Deserialize(serializedScene.dump());
		return sceneCopy;
	}

	json Scene::Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_VAL(Name);
		SERIALIZE_OBJECT(m_Environement)
		std::vector<json> entities = std::vector<json>();
		for (Entity e : GetAllEntities())
			entities.push_back(e.Serialize());
		SERIALIZE_VAL_LBL("Entities", entities);
		END_SERIALIZE();
	}

	bool Scene::Deserialize(const std::string& str)
	{
		if (str == "")
			return false;

		BEGIN_DESERIALIZE();
		if (!j.contains("Name"))
			return false;

		m_Registry.clear();

		Name = j["Name"];

		m_Environement = CreateRef<Environment>();
		if (j.contains("m_Environement"))
		{
			m_Environement = CreateRef<Environment>();
			std::string env = j["m_Environement"].dump();
			m_Environement->Deserialize(env);
		}

		// Parse entities
		if (!j.contains("Entities"))
			return 0;
		
		for (json e : j["Entities"])
		{
			std::string name = e["NameComponent"]["Name"];
			Entity ent = { m_Registry.create(), this };
			ent.Deserialize(e.dump());
		}

		auto view = m_Registry.view<ParentComponent>();
		for (auto e : view)
		{
			auto& parentComponent = view.get<ParentComponent>(e);
			if (!parentComponent.HasParent)
				continue;

			auto& entity = Entity{ e, this };
			auto parentEntity = GetEntityByID(parentComponent.ParentID);
			parentEntity.AddChild(entity);
		}

		return true;
	}
}

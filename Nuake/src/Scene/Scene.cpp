#pragma once
#include "Scene.h"
#include "Entities/Entity.h"

#include "src/Core/Core.h"
#include "src/Core/OS.h"
#include "src/Physics/PhysicsManager.h"

#include "src/Scene/Systems/ScriptingSystem.h"
#include "src/Scene/Systems/PhysicsSystem.h"
#include "src/Scene/Systems/TransformSystem.h"
#include "src/Scene/Systems/QuakeMapBuilder.h"
#include "src/Scene/Systems/ParticleSystem.h"
#include "src/Scene/Systems/AnimationSystem.h"
#include <src/Scene/Systems/AudioSystem.h>

#include "src/Rendering/SceneRenderer.h"
#include "src/Rendering/Renderer.h"
#include "src/Rendering/Textures/MaterialManager.h"

#include <glad/glad.h>

#include "Engine.h"
#include "src/Core/Maths.h"
#include "src/Core/FileSystem.h"
#include "src/Scene/Components/Components.h"
#include "src/Scene/Components/BoxCollider.h"
#include "src/Scene/Components/WrenScriptComponent.h"
#include "src/Scene/Components/BSPBrushComponent.h"
#include "src/Scene/Components/InterfaceComponent.h"
#include "src/Scene/Components/SkinnedModelComponent.h"
#include "src/Scene/Components/BoneComponent.h"

#include <fstream>
#include <future>
#include <streambuf>
#include <chrono>


namespace Nuake 
{
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
		m_Systems.push_back(CreateRef<PhysicsSystem>(this));
		m_Systems.push_back(CreateRef<ScriptingSystem>(this));
		m_Systems.push_back(CreateRef<AnimationSystem>(this));
		m_Systems.push_back(CreateRef<TransformSystem>(this));
		m_Systems.push_back(CreateRef<ParticleSystem>(this));
		m_Systems.push_back(CreateRef<AudioSystem>(this));

		m_SceneRenderer = CreateRef<SceneRenderer>();
		m_SceneRenderer->Init();
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
		if (m_EntitiesIDMap.find(id) != m_EntitiesIDMap.end())
		{
			return m_EntitiesIDMap[id];
		}

		auto idView = m_Registry.view<NameComponent>();
		for (auto e : idView) 
		{
			NameComponent& nameC = idView.get<NameComponent>(e);
			if (nameC.ID == id)
			{
				auto newEntity = Entity{ e, this };
				m_EntitiesIDMap[id] = newEntity;
				return newEntity;
			}
		}

		Logger::Log("Entity not found with id: " + std::to_string(id), "scene", CRITICAL);
		return Entity{ (entt::entity)0, this };
	}

	Entity Scene::GetEntityFromPath(const std::string& path)
	{
		auto splits = String::Split(path, '/');

		std::vector<Entity> rootEntities;
		const auto& view = m_Registry.view<ParentComponent, NameComponent>();
		for (const auto& e : view)
		{
			auto [parent, name] = view.get<ParentComponent, NameComponent>(e);
			if (!parent.HasParent)
			{
				rootEntities.push_back({ e, this });
			}
		}

		return Entity();
	}

	Entity Scene::GetRelativeEntityFromPath(Entity entity, const std::string& path)
	{
		return Entity();
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
		const auto& view = m_Registry.view<TransformComponent, CameraComponent, ParentComponent>();
		for (const auto& e : view)
		{
			auto [transform, camera, parent] = view.get<TransformComponent, CameraComponent, ParentComponent>(e);
			cam = camera.CameraInstance;

			cam->Translation = transform.GetGlobalPosition();
			break;
		}

		if (!cam)
		{
			return;
		}

		m_SceneRenderer->BeginRenderScene(cam->GetPerspective(), cam->GetTransform(), cam->Translation);
		m_SceneRenderer->RenderScene(*this, framebuffer);
	}

	void Scene::Draw(FrameBuffer& framebuffer, const Matrix4& projection, const Matrix4& view)
	{
		m_SceneRenderer->BeginRenderScene(m_EditorCamera->GetPerspective(), m_EditorCamera->GetTransform(), m_EditorCamera->Translation);
		m_SceneRenderer->RenderScene(*this, framebuffer);
	}
	
	std::vector<Entity> Scene::GetAllEntities() 
	{
		std::vector<Entity> allEntities;
		auto view = m_Registry.view<NameComponent>();
		for (auto& e : view) 
		{
			Entity newEntity(e, this);

			// Check if valid for deleted entities.
			if (newEntity.IsValid())
			{
				allEntities.push_back(newEntity);
			}
		}
		return allEntities;
	}

	Entity Scene::GetEntity(const std::string& name)
	{
		if (m_EntitiesNameMap.find(name) != m_EntitiesNameMap.end())
		{
			return m_EntitiesNameMap[name];
		}

		const auto& view = m_Registry.view<const NameComponent>();
		for (auto e : view) 
		{
			const auto& [namec] = view.get(e);
			if (namec.Name == name)
			{
				auto entity = Entity{ e, this };
				m_EntitiesNameMap[name] = entity;
				return entity;
			}
		}

		return Entity();
	}

	Entity Scene::CreateEntity(const std::string& name) 
	{
		return CreateEntity(name, (int)OS::GetTime());
	}

	std::string Scene::GetUniqueEntityName(const std::string& name)
	{
		std::string entityName;
		if (!EntityExists(name))
		{
			return name;
		}

		// Try to generate a unique name
		for (uint32_t i = 1; i < 4096; i++)
		{
			const std::string& entityEnumName = name + std::to_string(i);
			const auto& entityId = GetEntity(entityEnumName).GetHandle();
			if (entityId == -1)
			{
				return entityEnumName;
				break;
			}
		}

		// We ran out of names
		Logger::Log("Failed to create unique entity name. Limit reached with name: " + name, "scene", WARNING);
		return name;
	}

	Entity Scene::CreateEntity(const std::string& name, int id)
	{
		if (name.empty())
		{
			Logger::Log("Failed to create entity. Entity name cannot be empty.");
			return Entity();
		}

		std::string entityName = GetUniqueEntityName(name);
		
		Entity entity = { m_Registry.create(), this };

		// Add all mandatory component. An entity cannot exist without these.
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<ParentComponent>();
		entity.AddComponent<VisibilityComponent>();

		NameComponent& nameComponent = entity.AddComponent<NameComponent>();
		nameComponent.Name = entityName;
		nameComponent.ID = id;

		m_EntitiesIDMap[id] = entity;
		m_EntitiesNameMap[entityName] = entity;

		Logger::Log("Entity created with name: " + nameComponent.Name, "scene", LOG_TYPE::VERBOSE);
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
			DestroyEntity(c);
		}

		// Remove from ID to Entity cache
		if (m_EntitiesIDMap.find(entity.GetComponent<NameComponent>().ID) != m_EntitiesIDMap.end())
		{
			m_EntitiesIDMap.erase(entity.GetComponent<NameComponent>().ID);
		}

		if (m_EntitiesNameMap.find(entity.GetComponent<NameComponent>().Name) != m_EntitiesNameMap.end())
		{
			m_EntitiesNameMap.erase(entity.GetComponent<NameComponent>().Name);
		}

		entity.Destroy();
		m_Registry.shrink_to_fit();
	}

	bool Scene::EntityExists(const std::string& name)
	{
		return GetEntity(name).GetHandle() != -1;
	}

	Ref<Camera> Scene::GetCurrentCamera()
	{
		if (Engine::IsPlayMode())
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

		FileSystem::BeginWriteFile(path);
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
		sceneCopy->Path = Path;

		json serializedScene = Serialize();
		sceneCopy->Deserialize(serializedScene);
		return sceneCopy;
	}

	json Scene::Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_VAL(Name);
		SERIALIZE_OBJECT(m_Environement)
		SERIALIZE_VAL(Path)
		
		std::vector<json> entities = std::vector<json>();
		for (Entity e : GetAllEntities())
		{
			std::async(std::launch::async, [&]() {
					entities.push_back(e.Serialize());
			});
		}
		SERIALIZE_VAL_LBL("Entities", entities);

		SERIALIZE_OBJECT(m_EditorCamera);

		END_SERIALIZE();
	}

	bool Scene::Deserialize(const json& j)
	{
		if (j == "")
			return false;

		if (!j.contains("Name"))
			return false;

		m_Registry.clear();
		Name = j["Name"];

		if (j.contains(""))
		{
			Path = j["Path"];
		}

		m_Environement = CreateRef<Environment>();
		if (j.contains("m_Environement"))
		{
			m_Environement = CreateRef<Environment>();
			m_Environement->Deserialize(j["m_Environement"]);
		}

		// Parse entities
		if (!j.contains("Entities"))
		{
			return 0;
		}
		
		for (json e : j["Entities"])
		{
			std::string name = e["NameComponent"]["Name"];
			Entity ent = { m_Registry.create(), this };
			ent.Deserialize(e);
		}

		if (j.contains("m_EditorCamera"))
		{
			m_EditorCamera->Deserialize(j["m_EditorCamera"]);
		}

		auto view = m_Registry.view<ParentComponent>();
		for (auto e : view)
		{
			auto& parentComponent = view.get<ParentComponent>(e);
			if (!parentComponent.HasParent)
				continue;

			auto entity = Entity{ e, this };
			auto parentEntity = GetEntityByID(parentComponent.ParentID);
			parentEntity.AddChild(entity);
		}

		return true;
	}

	void Scene::CreateSkeleton(Entity& entity)
	{
		// We cannot create a component if the entity doesn't have a skinned model
		if (!entity.HasComponent<SkinnedModelComponent>())
		{
			const std::string msg = "Cannot create a skeleton on entity: " + std::to_string(entity.GetID());
			Logger::Log(msg);
			return;
		}

		auto& component = entity.GetComponent<SkinnedModelComponent>();
		auto& skeletonRoot = component.ModelResource->GetSkeletonRootNode();

		Entity skeletonRootEntity = CreateEntity(skeletonRoot.Name);
		skeletonRootEntity.AddComponent<BoneComponent>().Name = skeletonRoot.Name;
		skeletonRoot.EntityHandle = skeletonRootEntity.GetID();
		entity.AddChild(skeletonRootEntity);

		Vector3 bonePosition;
		Quat boneRotation;
		Vector3 boneScale;
		Decompose(skeletonRoot.Transform, bonePosition, boneRotation, boneScale);

		auto& transformComponent = skeletonRootEntity.GetComponent<TransformComponent>();
		transformComponent.SetLocalPosition(bonePosition);
		transformComponent.SetLocalRotation(boneRotation);
		transformComponent.SetLocalScale(boneScale);
		transformComponent.SetLocalTransform(skeletonRoot.Transform);

		CreateSkeletonTraverse(skeletonRootEntity, skeletonRoot);
	}

	void Scene::CreateSkeletonTraverse(Entity& entity, SkeletonNode& skeletonNode)
	{
		for (auto& c : skeletonNode.Children)
		{
			Entity boneEntity = CreateEntity(c.Name);
			boneEntity.AddComponent<BoneComponent>();
			entity.AddChild(boneEntity);

			c.EntityHandle = boneEntity.GetID();

			Vector3 bonePosition;
			Quat boneRotation;
			Vector3 boneScale;
			Decompose(c.Transform, bonePosition, boneRotation, boneScale);

			auto& transformComponent = boneEntity.GetComponent<TransformComponent>();
			transformComponent.SetLocalPosition(bonePosition);
			transformComponent.SetLocalRotation(boneRotation);
			transformComponent.SetLocalScale(boneScale);
			transformComponent.SetLocalTransform(c.Transform);
			//transformComponent.Dirty = false;

			CreateSkeletonTraverse(boneEntity, c);
		}
	}

}

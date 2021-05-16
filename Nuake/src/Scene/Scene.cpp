#pragma once
#include "Scene.h"
#include "Entities/Entity.h"
#include "Entities/Components.h"
#include "../Rendering/Renderer.h"
#include "../Core/MaterialManager.h"
#include "../Core/Physics/PhysicsManager.h"
#include "../Core/Core.h"
#include <GL/glew.h>
#include "Entities/Components/BoxCollider.h"
#include "../../Engine.h"
#include "../Core/FileSystem.h"

#include <fstream>
#include <streambuf>

Ref<Scene> Scene::New()
{
	return CreateRef<Scene>();
}

Scene::Scene()
{
	auto camEntity = CreateEntity("Camera");
	camEntity.AddComponent<CameraComponent>().transformComponent = &camEntity.GetComponent<TransformComponent>();

	m_EditorCamera = CreateRef<EditorCamera>();
	m_Environement = CreateRef<Environment>();
}

Scene::~Scene() {}

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

void Scene::OnInit()
{
	// Create physic world.
	auto view = m_Registry.view<TransformComponent, RigidBodyComponent>();
	for (auto e : view)
	{
		auto [transform, rigidbody] = view.get<TransformComponent, RigidBodyComponent>(e);

		Entity ent = Entity({ e, this });
		
		if (ent.HasComponent<BoxColliderComponent>())
		{
			float mass = rigidbody.mass;

			BoxColliderComponent& boxComponent = ent.GetComponent<BoxColliderComponent>();
			Ref<Physics::Box> boxShape = CreateRef<Physics::Box>(boxComponent.Size);

			Ref<Physics::RigidBody> btRigidbody = CreateRef<Physics::RigidBody>(mass, transform.Translation, boxShape);
			rigidbody.m_Rigidbody = btRigidbody;

			btRigidbody->SetKinematic(rigidbody.IsKinematic);

			PhysicsManager::Get()->RegisterBody(btRigidbody);
		}
	}

	// character controllers
	auto ccview = m_Registry.view<TransformComponent, CharacterControllerComponent>();
	for (auto e : ccview)
	{
		auto [transform, cc] = ccview.get<TransformComponent, CharacterControllerComponent>(e);

		cc.CharacterController = CreateRef<Physics::CharacterController>(cc.Height, cc.Radius, cc.Mass);
		Entity ent = Entity({ e, this });

		PhysicsManager::Get()->RegisterCharacterController(cc.CharacterController);
	}

	auto quakeMapview = m_Registry.view<TransformComponent, QuakeMap>();
	for (auto e : quakeMapview)
	{
		auto [transform, quake] = quakeMapview.get<TransformComponent, QuakeMap>(e);

		if (quake.HasCollisions)
		{
			for (auto m : quake.m_Meshes)
			{
				Ref<Physics::MeshShape> meshShape = CreateRef<Physics::MeshShape>(m);
				Ref<Physics::RigidBody> btRigidbody = CreateRef<Physics::RigidBody>(0.0f, transform.Translation, meshShape);
				PhysicsManager::Get()->RegisterBody(btRigidbody);
			}
		}
	}

	// Instanciate scripts.
	{
		m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
		{
			if (!nsc.Instance)
			{
				nsc.Instance = nsc.InstantiateScript();
				nsc.Instance->m_Entity = Entity{ entity, this };
			}
			nsc.Instance->OnCreate();
		});
	}
}

void Scene::OnExit()
{
	PhysicsManager::Get()->Reset();

	// destroy scripts.
	{
		m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
		{
			nsc.Instance->OnDestroy();
		});
	}
}

// update entities and some components.
void Scene::Update(Timestep ts)
{
	// Update scripts
	{
		m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
		{
			nsc.Instance->OnUpdate(ts);
		});
	}

	// Update rigidbodies
	PhysicsManager::Get()->Step(ts);

	auto physicGroup = m_Registry.view<TransformComponent, RigidBodyComponent>();
	for (auto e : physicGroup) {
		auto [transform, rb] = physicGroup.get<TransformComponent, RigidBodyComponent>(e);
		rb.SyncTransformComponent(&m_Registry.get<TransformComponent>(e));
	}

	auto ccGroup = m_Registry.view<TransformComponent, CharacterControllerComponent>();
	for (auto e : ccGroup) {
		auto [transform, rb] = ccGroup.get<TransformComponent, CharacterControllerComponent>(e);
		rb.SyncWithTransform(m_Registry.get<TransformComponent>(e));
	}
}


void Scene::EditorUpdate(Timestep ts)
{
	m_EditorCamera->Update(ts);
}


void Scene::DrawShadows()
{

	auto modelView = m_Registry.view<TransformComponent, ModelComponent>();
	auto quakeView = m_Registry.view<TransformComponent, QuakeMap>();
	auto view = m_Registry.view<TransformComponent, LightComponent>();

	Ref<Camera> cam = nullptr;
	if (Engine::IsPlayMode)
	{
		auto view = m_Registry.view<TransformComponent, CameraComponent>();
		for (auto e : view) {
			auto [transform, camera] = view.get<TransformComponent, CameraComponent>(e);
			cam = camera.CameraInstance;
			break;
		}
	}
	else
	{
		cam = m_EditorCamera;
	}

	glm::mat4 perspective = cam->GetPerspective();

	for (auto l : view) {
		auto [lightTransform, light] = view.get<TransformComponent, LightComponent>(l);
		if (light.Type != LightType::Directional)
			continue;
        
		light.BeginDrawShadow();
		for (auto e : modelView)
		{
			auto [transform, model] = modelView.get<TransformComponent, ModelComponent>(e);
            
			glm::vec3 pos = lightTransform.Translation;
			glm::mat4 lightView = glm::lookAt(pos, pos - light.GetDirection(), glm::vec3(0.0f, 1.0f, 0.0f));
			Renderer::m_ShadowmapShader->SetUniformMat4f("lightSpaceMatrix", light.GetProjection() * lightView);
			Renderer::m_ShadowmapShader->SetUniformMat4f("model", transform.GetTransform());
            
			model.Draw();
		}

		for (auto e : quakeView) {
			auto [transform, model] = quakeView.get<TransformComponent, QuakeMap>(e);

			glm::vec3 pos = lightTransform.Translation;
			glm::mat4 lightView = glm::lookAt(pos, pos - light.GetDirection(), glm::vec3(0.0f, 1.0f, 0.0f));

			Renderer::m_ShadowmapShader->SetUniformMat4f("lightSpaceMatrix", light.GetProjection() * lightView);
			Renderer::m_ShadowmapShader->SetUniformMat4f("model", transform.GetTransform());

			model.Draw();
		}
		light.EndDrawShadow();
	}
}


void Scene::Draw()
{
	// Find the camera of the scene.
	Ref<Camera> cam = nullptr;
	{
		auto view = m_Registry.view<TransformComponent, CameraComponent, ParentComponent>();
		for (auto e : view) {
			auto [transform, camera, parent] = view.get<TransformComponent, CameraComponent, ParentComponent>(e);

			TransformComponent copyT = transform;
			if (parent.HasParent)
			{
				Entity currentParent = Entity{ e, this };
				glm::vec3 globalOffset = copyT.Translation;

				while (currentParent.GetComponent<ParentComponent>().HasParent)
				{
					currentParent = currentParent.GetComponent<ParentComponent>().Parent;
					globalOffset += currentParent.GetComponent<TransformComponent>().Translation;
				}

				copyT.Translation = globalOffset;
			}

			cam = camera.CameraInstance;
			cam->Translation = copyT.Translation;
			break;
		}
	}
	glDisable(GL_DEPTH_TEST);
	Ref<Environment> env = GetEnvironment();

	if (env->ProceduralSkybox)
	{
		env->ProceduralSkybox->Draw(cam);

	}
	Renderer::m_Shader->Bind();
	env->Push();
	glEnable(GL_DEPTH_TEST);

	// Push lights
	{
		auto view = m_Registry.view<TransformComponent, LightComponent, ParentComponent>();
		for (auto l : view) {
			auto [transform, light, parent] = view.get<TransformComponent, LightComponent, ParentComponent>(l);

			TransformComponent copyT = transform;
			if (parent.HasParent)
			{
				Entity currentParent = Entity{ l, this };
				glm::vec3 globalOffset = copyT.Translation;

				while (currentParent.GetComponent<ParentComponent>().HasParent)
				{
					currentParent = currentParent.GetComponent<ParentComponent>().Parent;
					globalOffset += currentParent.GetComponent<TransformComponent>().Translation;

				}

				copyT.Translation = globalOffset;
			}

			if (light.SyncDirectionWithSky)
				light.Direction = GetEnvironment()->ProceduralSkybox->GetSunDirection();

			light.Draw(copyT, m_EditorCamera);
		}
	}

	Renderer::m_Shader->Bind();

	Renderer::m_Shader->SetUniform1i("u_ShowNormal", 0);
	if (cam)
	{
		Renderer::m_Shader->SetUniform1f("u_Exposure", cam->Exposure);

		auto view = m_Registry.view<TransformComponent, ModelComponent, ParentComponent>();
		for (auto e : view) {
			auto [transform, model, parent] = view.get<TransformComponent, ModelComponent, ParentComponent>(e);
			TransformComponent copyT = transform;
			if (parent.HasParent)
			{
				Entity currentParent = Entity{ e, this };
				glm::vec3 globalOffset = copyT.Translation;

				while (currentParent.GetComponent<ParentComponent>().HasParent)
				{
					currentParent = currentParent.GetComponent<ParentComponent>().Parent;
					globalOffset += currentParent.GetComponent<TransformComponent>().Translation;
				}

				copyT.Translation = globalOffset;
			}
			Renderer::m_Shader->SetUniformMat4f("u_View", cam->GetTransform());
			Renderer::m_Shader->SetUniformMat4f("u_Projection", cam->GetPerspective());
			Renderer::m_Shader->SetUniformMat4f("u_Model", copyT.GetTransform());
			model.Draw();
		}

		auto quakeView = m_Registry.view<TransformComponent, QuakeMap, ParentComponent>();
		for (auto e : quakeView) {
			auto [transform, model, parent] = quakeView.get<TransformComponent, QuakeMap, ParentComponent>(e);

			TransformComponent copyT = transform;
			if (parent.HasParent)
			{
				Entity currentParent = Entity{ e, this };
				glm::vec3 globalOffset = copyT.Translation;

				while (currentParent.GetComponent<ParentComponent>().HasParent)
				{
					currentParent = currentParent.GetComponent<ParentComponent>().Parent;
					globalOffset += currentParent.GetComponent<TransformComponent>().Translation;
				}

				copyT.Translation = globalOffset;
			}
			Renderer::m_Shader->SetUniformMat4f("u_View", cam->GetTransform());
			Renderer::m_Shader->SetUniformMat4f("u_Projection", cam->GetPerspective());
			Renderer::m_Shader->SetUniformMat4f("u_Model", copyT.GetTransform());
			model.Draw();
		}

		Renderer::m_DebugShader->SetUniformMat4f("u_View", cam->GetTransform());
		Renderer::m_DebugShader->SetUniformMat4f("u_Projection", cam->GetPerspective());
		PhysicsManager::Get()->DrawDebug();
	}
}

void Scene::EditorDraw()
{
	glDisable(GL_DEPTH_TEST);
	Ref<Environment> env = GetEnvironment();

	if (env->ProceduralSkybox)
	{
		env->ProceduralSkybox->Draw(m_EditorCamera);
	}

	Renderer::m_Shader->Bind();
	env->Push();
	glEnable(GL_DEPTH_TEST);

	// Push lights
	{
		auto view = m_Registry.view<TransformComponent, LightComponent, ParentComponent>();
		for (auto l : view) {
			auto [transform, light, parent] = view.get<TransformComponent, LightComponent, ParentComponent>(l);
			
			TransformComponent copyT = transform;
			if (parent.HasParent)
			{
				Entity currentParent = Entity{ l, this };
				glm::vec3 globalOffset = copyT.Translation;

				while (currentParent.GetComponent<ParentComponent>().HasParent)
				{
					currentParent = currentParent.GetComponent<ParentComponent>().Parent;
					globalOffset += currentParent.GetComponent<TransformComponent>().Translation;
				}

				copyT.Translation = globalOffset;
			}

			if (light.SyncDirectionWithSky)
				light.Direction = GetEnvironment()->ProceduralSkybox->GetSunDirection();

			light.Draw(copyT, m_EditorCamera);
		}
	}

	Renderer::m_Shader->Bind();
	Renderer::m_Shader->SetUniform1i("u_ShowNormal", 0);
	if (m_EditorCamera)
	{
		Renderer::m_Shader->SetUniform1f("u_Exposure", m_EditorCamera->Exposure);


		

		auto view = m_Registry.view<TransformComponent, ModelComponent, ParentComponent>();
		for (auto e : view) {
			auto [transform, model, parent] = view.get<TransformComponent, ModelComponent, ParentComponent>(e);

			TransformComponent copyT = transform;
			if (parent.HasParent)
			{
				Entity currentParent = Entity{ e, this };
				glm::vec3 globalOffset = copyT.Translation;

				while (currentParent.GetComponent<ParentComponent>().HasParent)
				{
					currentParent = currentParent.GetComponent<ParentComponent>().Parent;
					globalOffset += currentParent.GetComponent<TransformComponent>().Translation;
				}

				copyT.Translation = globalOffset;
			}

			Renderer::m_Shader->SetUniformMat4f("u_View", m_EditorCamera->GetTransform());
			Renderer::m_Shader->SetUniformMat4f("u_Projection", m_EditorCamera->GetPerspective());
			Renderer::m_Shader->SetUniformMat4f("u_Model", copyT.GetTransform());
			model.Draw();
		}

		auto quakeView = m_Registry.view<TransformComponent, QuakeMap, ParentComponent>();
		for (auto e : quakeView) {
			auto [transform, model, parent] = quakeView.get<TransformComponent, QuakeMap, ParentComponent>(e);


			TransformComponent copyT = transform;
			if (parent.HasParent)
			{
				Entity currentParent = Entity{ e, this };
				glm::vec3 globalOffset = copyT.Translation;

				while (currentParent.GetComponent<ParentComponent>().HasParent)
				{
					currentParent = currentParent.GetComponent<ParentComponent>().Parent;
					globalOffset += currentParent.GetComponent<TransformComponent>().Translation;
				}

				copyT.Translation = globalOffset;
			}
			Renderer::m_Shader->SetUniformMat4f("u_View", m_EditorCamera->GetTransform());
			Renderer::m_Shader->SetUniformMat4f("u_Projection", m_EditorCamera->GetPerspective());

			Renderer::m_Shader->SetUniformMat4f("u_Model", copyT.GetTransform());
			model.Draw();
		}

		auto boxCollider = m_Registry.view<TransformComponent, BoxColliderComponent, ParentComponent>();
		for (auto e : boxCollider) {
			auto [transform, box, parent] = boxCollider.get<TransformComponent, BoxColliderComponent, ParentComponent>(e);

			TransformComponent copyT = transform;
			if (parent.HasParent)
			{
				Entity currentParent = Entity{ e, this };
				glm::vec3 globalOffset = copyT.Translation;

				while (currentParent.GetComponent<ParentComponent>().HasParent)
				{
					currentParent = currentParent.GetComponent<ParentComponent>().Parent;
					globalOffset += currentParent.GetComponent<TransformComponent>().Translation;

				}

				copyT.Translation = globalOffset;
			}

			Renderer::m_DebugShader->SetUniformMat4f("u_View", m_EditorCamera->GetTransform());
			Renderer::m_DebugShader->SetUniformMat4f("u_Projection", m_EditorCamera->GetPerspective());
			Renderer::m_DebugShader->SetUniformMat4f("u_Model", copyT.GetTransform());
			
			TransformComponent t = transform;
			t.Scale = (box.Size * 2.f) * transform.Scale;
			Renderer::DrawCube(t, glm::vec4(0.0f, 0.0f, 0.9f, 0.2f));
		}

		auto sphereCollider = m_Registry.view<TransformComponent, SphereColliderComponent, ParentComponent>();
		for (auto e : sphereCollider) {
			auto [transform, box, parent] = sphereCollider.get<TransformComponent, SphereColliderComponent, ParentComponent>(e);

			TransformComponent copyT = transform;
			if (parent.HasParent)
			{
				Entity currentParent = Entity{ e, this };
				glm::vec3 globalOffset = copyT.Translation;

				while (currentParent.GetComponent<ParentComponent>().HasParent)
				{
					currentParent = currentParent.GetComponent<ParentComponent>().Parent;
					globalOffset += currentParent.GetComponent<TransformComponent>().Translation;

				}

				copyT.Translation = globalOffset;
			}

			Renderer::m_DebugShader->SetUniformMat4f("u_View", m_EditorCamera->GetTransform());
			Renderer::m_DebugShader->SetUniformMat4f("u_Projection", m_EditorCamera->GetPerspective());
			Renderer::m_DebugShader->SetUniformMat4f("u_Model", transform.GetTransform());

			TransformComponent t = transform;
			t.Scale = (box.Radius * 2.f) * transform.Scale;
			Renderer::DrawSphere(t, glm::vec4(0.0f, 0.0f, 0.9f, 0.2f));
		}
	}
}

std::vector<Entity> Scene::GetAllEntities() {
	std::vector<Entity> allEntities;
	auto view = m_Registry.view<TransformComponent, NameComponent>();
	for (auto e : view) {
		allEntities.push_back(Entity(e, this));
	}
	return allEntities;
}

glm::vec3 Scene::GetGlobalPosition(Entity ent)
{
	glm::vec3 globalPos = ent.GetComponent<TransformComponent>().Translation;
	Entity currentParent = ent.AddComponent<ParentComponent>().Parent;
	while (currentParent.GetHandle() != -1)
	{
		globalPos += currentParent.GetComponent<TransformComponent>().Translation;
		currentParent = currentParent.GetComponent<ParentComponent>().Parent;
	}

	return globalPos;
}


Entity Scene::GetEntity(const std::string& name)
{
	std::vector<Entity> allEntities;
	auto view = m_Registry.view<TransformComponent, NameComponent>();
	for (auto e : view) {
		auto [transform, namec] = view.get<TransformComponent, NameComponent>(e);
		if (namec.Name == name)
			return Entity{ e, this };
	}
}

Entity Scene::CreateEntity(const std::string& name) {
	Entity entity = { m_Registry.create(), this };
    
	// Must have transform
	entity.AddComponent<TransformComponent>();
    
	NameComponent& nameComponent = entity.AddComponent<NameComponent>();
	nameComponent.Name = name;

	ParentComponent& parentComponent = entity.AddComponent<ParentComponent>();

	std::string str = "Created entity: " + nameComponent.Name + "\n";
	printf(str.c_str());
	return entity;
}

void Scene::DestroyEntity(Entity entity) {
	entity.Destroy();
}

// Getter
Ref<Camera> Scene::GetCurrentCamera()
{
	if (Engine::IsPlayMode)
	{
		Ref<Camera> cam = nullptr;
		{
			auto view = m_Registry.view<TransformComponent, CameraComponent>();
			for (auto e : view) {
				auto [transform, camera] = view.get<TransformComponent, CameraComponent>(e);
				cam = camera.CameraInstance;
				break;
			}
		}
		return cam;
	}

	return m_EditorCamera;
}

Ref<Environment> Scene::GetEnvironment() {
	return m_Environement;
}

bool Scene::Save()
{
	if (Path == "")
		Path = FileDialog::SaveFile("*.scene") + ".scene";

	return SaveAs(Path);
}

bool Scene::SaveAs(const std::string& path)
{
	std::ofstream sceneFile;
	sceneFile.open(path);
	sceneFile << Serialize().dump(4);
	sceneFile.close();
	return true;
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
	return false;
}

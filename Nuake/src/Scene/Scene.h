#pragma once
#include "../Rendering/Camera.h"
#include "Lighting/Environment.h"
#include <vector>
#include "entt/entt.hpp"
#include "../Rendering/SkyboxHDR.h"
#include "../Rendering/GBuffer.h"
#include "../Rendering/ProceduralSky.h"
#include "../Core/Core.h"
#include "EditorCamera.h"

class Entity;
class __declspec(dllexport) Scene
{
	friend Entity;

private:
	Ref<Environment> m_Environement;
	entt::registry m_Registry;
	Ref<EditorCamera> m_EditorCamera;

public:

	SkyboxHDR* m_Skybox;
	Scene();
	~Scene();

	void Init();
	void OnInit();
	void OnExit();
	void Update(Timestep ts);
	void EditorUpdate(Timestep ts);

	void DrawShadows();
	void DrawGBuffer();
	void DrawDeferred();
	void Draw();
	void EditorDraw();

	std::vector<Entity> GetAllEntities();
	glm::vec3 GetGlobalPosition(Entity ent);
	Entity GetEntity(const std::string name);
	Entity CreateEntity(const std::string name);
	void DestroyEntity(Entity entity);

	Ref<Camera> GetCurrentCamera();
	Ref<Environment> GetEnvironment();
};
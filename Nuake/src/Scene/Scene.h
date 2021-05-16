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
#include "../Resource/Serializable.h"

class Entity;
class Scene : public ISerializable
{
	friend Entity;

private:
	Ref<Environment> m_Environement;
	entt::registry m_Registry;
	Ref<EditorCamera> m_EditorCamera;
	std::string Name;
	std::string Path = "";
	bool has_changed = true;
public:
	static Ref<Scene> New();
	Scene();
	~Scene();

	std::string GetName();
	bool SetName(std::string& newName);

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
	Entity GetEntity(const std::string& name);
	Entity CreateEntity(const std::string& name);
	void DestroyEntity(Entity entity);

	Ref<Camera> GetCurrentCamera();
	Ref<Environment> GetEnvironment();

	bool Save();
	bool SaveAs(const std::string& path);


	json Serialize() override;
	bool Deserialize(const std::string& str) override;
};
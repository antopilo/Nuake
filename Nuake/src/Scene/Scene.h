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
#include "src/UI/UserInterface.h"
#include "../Core/Maths.h"
#include <src/Scene/Systems/System.h>

class Entity;

class Scene : public ISerializable
{
	friend Entity;
private:
	std::string Name; // Name of the scene
	bool has_changed = true;

	// The systems are what updates the components.
	// You can create a new system(see 'Systems/'.) and register it
	// In the scene constructor.
	std::vector<Ref<System>> m_Systems;
	
	Ref<Environment> m_Environement;
public:
	Ref<EditorCamera> m_EditorCamera;
	entt::registry m_Registry;
	std::vector<Ref<UI::UserInterface>> m_Interfaces;
	std::string Path = "";
	static Ref<Scene> New();

	Scene();
	~Scene();

	std::string GetName();
	bool SetName(std::string& newName);
	Entity GetEntity(int handle);
	Entity GetEntityByID(int id);
	void Init();
	bool OnInit();
	void OnExit();
	void Update(Timestep ts);
	void FixedUpdate(Timestep ts);
	void EditorUpdate(Timestep ts);

	void UpdatePositions();

	// TODO: Maybe move this to Renderer::DrawScene() ?
	void DrawShadows();
	void DrawInterface(Vector2 screensize);
	void Draw();
	void EditorDraw();

	std::vector<Entity> GetAllEntities();
	Entity GetEntity(const std::string& name);

	// TODO: This shouldnt be allowed
	Entity CreateEmptyEntity();
	Entity CreateEntity(const std::string& name);

	// TODO: Could be moved to transform component directly.
	glm::vec3 GetGlobalPosition(Entity ent);
	void DestroyEntity(Entity entity); // TODO: Could return bool

	// TODO: Add multiple camera support.
	Ref<Camera> GetCurrentCamera();
	Ref<Environment> GetEnvironment();

	bool Save();
	bool SaveAs(const std::string& path);

	void ReloadInterfaces();
	void AddInterface(Ref<UI::UserInterface> interface);

	json Serialize() override;
	bool Deserialize(const std::string& str) override;

	void Snapshot();
	void LoadSnapshot();
};
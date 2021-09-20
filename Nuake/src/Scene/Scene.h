#pragma once
#include <vector>
#include "src/Core/Core.h"
#include "src/Core/Maths.h"

#include "entt/entt.hpp"

#include "src/Rendering/Camera.h"
#include "Lighting/Environment.h"

#include "EditorCamera.h"
#include "src/Resource/Serializable.h"
#include "src/UI/UserInterface.h"

#include "src/Scene/Systems/System.h"

namespace Nuake {
	class Entity;
	class SceneRenderer;
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
		SceneRenderer* mSceneRenderer;
	public:
		Ref<EditorCamera> m_EditorCamera;
		entt::registry m_Registry;
		std::string Path = "";

		static Ref<Scene> New();
		Scene();
		~Scene();

		std::string GetName();
		bool SetName(std::string& newName);

		Entity GetEntity(int handle);
		Entity GetEntityByID(int id);

		bool OnInit();
		void OnExit();
		void Update(Timestep ts);
		void FixedUpdate(Timestep ts);
		void EditorUpdate(Timestep ts);

		void Draw(FrameBuffer& framebuffer);
		void Draw(FrameBuffer& framebuffer, const Matrix4& projection, const Matrix4& view);

		std::vector<Entity> GetAllEntities();
		Entity GetEntity(const std::string& name);

		Entity CreateEntity(const std::string& name);
		void DestroyEntity(Entity entity); // TODO: Could return bool

		Ref<Camera> GetCurrentCamera();
		Ref<Environment> GetEnvironment() const;

		bool Save();
		bool SaveAs(const std::string& path);

		void AddInterface(Ref<UI::UserInterface> interface);

		json Serialize() override;
		bool Deserialize(const std::string& str) override;
	};
}

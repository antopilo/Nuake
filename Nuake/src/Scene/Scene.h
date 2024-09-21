#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"

#include <entt/entt.hpp>

#include "src/Rendering/Camera.h"
#include "Lighting/Environment.h"

#include "EditorCamera.h"
#include "src/Resource/Serializable.h"
#include "src/Scene/Systems/System.h"
#include "src/Resource/UUID.h"

namespace Nuake 
{
	struct SkeletonNode;

	class Entity;
	class SceneRenderer;

	DECLARE_MULTICAST_DELEGATE(PreInitializeDelegate)
	DECLARE_MULTICAST_DELEGATE(PostInitializeDelegate)

	class PhysicsSystem;
	class ScriptingSystem;

	class Scene : public ISerializable
	{
		friend Entity;

	private:
		std::string Name;
		bool has_changed = true;

		// The systems are what updates the components.
		// You can create a new system(see 'Systems/'.) and register it
		// In the scene constructor.
		std::vector<Ref<System>> m_Systems;
		Ref<Environment> m_Environement;

	public:
		Ref<EditorCamera> m_EditorCamera;
		entt::registry m_Registry;
		std::unordered_map<uint32_t, Entity> m_EntitiesIDMap;
		std::unordered_map<std::string, Entity> m_EntitiesNameMap;
		std::string Path = "";

		Ref<PhysicsSystem> physicsSystem;
		Ref<ScriptingSystem> scriptingSystem;

		Ref<SceneRenderer> m_SceneRenderer;

		static Ref<Scene> New();
		Scene();
		~Scene();

		bool OnInit();
		void OnExit();
		void Update(Timestep ts);
		void FixedUpdate(Timestep ts);
		void EditorUpdate(Timestep ts);

		void Draw(FrameBuffer& framebuffer);
		void Draw(FrameBuffer& framebuffer, const Matrix4& projection, const Matrix4& view);

		std::string GetName();
		bool SetName(const std::string& newName);

		Ref<Camera> GetCurrentCamera();

		std::string GetUniqueEntityName(const std::string& name);

		Entity CreateEntity(const std::string& name);
		Entity CreateEntity(const std::string& name, int id);
		void DestroyEntity(Entity entity);		
		bool EntityExists(const std::string& name);

		std::vector<Entity> GetAllEntities();
		Entity GetEntity(const std::string& name);
		Entity GetEntity(int handle);
		Entity GetEntityByID(int id);
		Entity GetEntityFromPath(const std::string& path);
		Entity GetRelativeEntityFromPath(Entity entity, const std::string& path);

		bool EntityIsParent(Entity entity, Entity parent);

		template<typename Component>
		static void CopyComponent(entt::registry& dst, entt::registry& src);

		Ref<Environment> GetEnvironment() const;
		void SetEnvironment(Ref<Environment> env);

		bool Save();
		bool SaveAs(const std::string& path);
		Ref<Scene> Copy();

		json Serialize() override;
		bool Deserialize(const json& j) override;

		// Component specific utilies
		void CreateSkeleton(Entity& entity);

		PreInitializeDelegate& OnPreInitialize() { return preInitializeDelegate; }
		PostInitializeDelegate& OnPostInitialize() { return postInitializeDelegate; }

		Ref<PhysicsSystem> GetPhysicsSystem() const { return physicsSystem; }
		Ref<ScriptingSystem> GetScriptingSystem() const { return scriptingSystem; }

	protected:
		PreInitializeDelegate preInitializeDelegate;
		PostInitializeDelegate postInitializeDelegate;
		
	private:
		void CreateSkeletonTraverse(Entity& entity, SkeletonNode& skeletonNode);
	};
}

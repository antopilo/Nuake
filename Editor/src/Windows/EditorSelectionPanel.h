#pragma once
#include "../Actions/EditorSelection.h"
#include "src/Scene/Entities/Entity.h"

#include "src/FileSystem/FileSystem.h"
#include <src/Resource/Project.h>

#include "../ComponentsPanel/TransformPanel.h"
#include "../ComponentsPanel/LightPanel.h"
#include "../ComponentsPanel/MeshPanel.h"
#include "../ComponentsPanel/CameraPanel.h"
#include "../ComponentsPanel/CapsuleColliderPanel.h"
#include "../ComponentsPanel/CylinderColliderPanel.h"
#include "../ComponentsPanel/MeshColliderPanel.h"
#include "../ComponentsPanel/CharacterControllerPanel.h"
#include "../ComponentsPanel/BonePanel.h"
#include "../ComponentsPanel/NetScriptPanel.h"
#include "../ComponentsPanel/NavMeshVolumePanel.h"

#include <src/Scene/Components/WrenScriptComponent.h>
#include <src/Resource/Prefab.h>


namespace Nuake
{
	class Scene;
}

class EditorSelectionPanel 
{
	using DrawComponentTypeFn = std::function<void(Nuake::Entity& entity, entt::meta_any& componentInstance)>;
	using DrawFieldTypeFn = std::function<void(entt::meta_data& fieldMeta, entt::meta_any& componentInstance)>;
	
private:
	TransformPanel mTransformPanel;
	LightPanel mLightPanel;
	NetScriptPanel mNetScriptPanel;
	MeshPanel mMeshPanel;
	CameraPanel mCameraPanel;
	MeshColliderPanel mMeshColliderPanel;
	CapsuleColliderPanel mCapsuleColliderPanel;
	CylinderColliderPanel mCylinderColliderPanel;
	CharacterControllerPanel mCharacterControllerPanel;
	BonePanel mBonePanel;
	NavMeshVolumePanel mNavMeshVolumePanel;

	Ref<Nuake::File> currentFile;
	Ref<Nuake::Resource> selectedResource;

	Ref<Nuake::Scene> virtualScene;
public:
	EditorSelectionPanel();

	void Draw(EditorSelection selection);

	void DrawNone();
	void DrawEntity(Nuake::Entity entity);
	void DrawAddComponentMenu(Nuake::Entity entity);

	void DrawFile(Ref<Nuake::File> file);
	void DrawResource(Nuake::Resource resource);
	void DrawPrefabPanel(Ref<Nuake::Prefab> prefab);

	template<class T, auto Func>
	void RegisterComponentDrawer()
	{
		const auto t = entt::type_id<T>();
		ComponentTypeDrawers[t.hash()] = std::bind(Func, std::placeholders::_1, std::placeholders::_2);
	}

	template<class T, auto Func, class O>
	void RegisterComponentDrawer(O* o)
	{
		ComponentTypeDrawers[entt::type_id<T>().hash()] = std::bind(Func, o, std::placeholders::_1, std::placeholders::_2);
	}

	template<class T, auto Func, class O>
	void RegisterTypeDrawer(O* o)
	{
		FieldTypeDrawers[entt::type_id<T>().hash()] = std::bind(Func, o, std::placeholders::_1, std::placeholders::_2);
	}

protected:
	// Drawing functions for each component (for writing very specific inspectors for specific components)
	std::unordered_map<entt::id_type, DrawComponentTypeFn> ComponentTypeDrawers;
	
	// List of functions to call for each component field type that needs to be drawn
	std::unordered_map<entt::id_type, DrawFieldTypeFn> FieldTypeDrawers;
	
private:
	void ResolveFile(Ref<Nuake::File> file);
	void DrawMaterialPanel(Ref<Nuake::Material> material);
	void DrawProjectPanel(Ref<Nuake::Project> project);
	void DrawWrenScriptPanel(Ref<Nuake::WrenScript> wrenFile);
	void DrawNetScriptPanel(Ref<Nuake::File> file);

	void DrawComponent(Nuake::Entity& entity, entt::meta_any& component);
	void DrawComponentContent(entt::meta_any& component);

	void DrawFieldTypeFloat(entt::meta_data& field, entt::meta_any& component);
	void DrawFieldTypeBool(entt::meta_data& field, entt::meta_any& component);
	void DrawFieldTypeVector3(entt::meta_data& field, entt::meta_any& component);
	void DrawFieldTypeString(entt::meta_data& field, entt::meta_any& component);
	void DrawFieldTypeResourceFile(entt::meta_data& field, entt::meta_any& component);
	void DrawFieldTypeDynamicItemList(entt::meta_data& field, entt::meta_any& component);
};
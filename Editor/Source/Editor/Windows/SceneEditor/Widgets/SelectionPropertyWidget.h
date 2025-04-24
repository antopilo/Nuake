#pragma once
#include "IEditorWidget.h"
#include "../../EditorSelectionPanel.h"

#include "../../../misc/AnimatedValue.h"

class EditorContext;

using DrawComponentTypeFn = std::function<void(Nuake::Entity& entity, entt::meta_any& componentInstance)>;
using DrawFieldTypeFn = std::function<void(entt::meta_data& fieldMeta, entt::meta_any& componentInstance)>;

class SelectionPropertyWidget : public IEditorWidget
{
private:
	TransformPanel transformPanel;
	MeshPanel meshPanel;
	SkinnedMeshPanel skinnedMeshPanel;
	Ref<Nuake::File> currentFile;
	Ref<Nuake::Resource> selectedResource;
	AnimatedValue<float> opacity;

public:
	SelectionPropertyWidget(EditorContext& inCtx);
	~SelectionPropertyWidget() = default;

public:
	void Update(float ts) override;
	void Draw() override;

private:
	void DrawNone();
	void DrawEntity(Nuake::Entity entity);
	void DrawAddComponentMenu(Nuake::Entity entity);
	void DrawFile(Ref<Nuake::File> file);
	void DrawResource(Nuake::Resource resource);

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

	void ResolveFile(Ref<Nuake::File> file);
	void DrawMaterialPanel(Ref<Nuake::Material> material);
	void DrawProjectPanel(Ref<Nuake::Project> project);
	void DrawNetScriptPanel(Ref<Nuake::File> file);

	void DrawComponent(Nuake::Entity& entity, entt::meta_any& component);
	void DrawComponentContent(entt::meta_any& component);

	void DrawFieldTypeFloat(entt::meta_data& field, entt::meta_any& component);
	void DrawFieldTypeBool(entt::meta_data& field, entt::meta_any& component);
	void DrawFieldTypeVector2(entt::meta_data& field, entt::meta_any& component);
	void DrawFieldTypeVector3(entt::meta_data& field, entt::meta_any& component);
	void DrawFieldTypeString(entt::meta_data& field, entt::meta_any& component);
	void DrawFieldTypeResourceFile(entt::meta_data& field, entt::meta_any& component);
	void DrawFieldTypeDynamicItemList(entt::meta_data& field, entt::meta_any& component);
};
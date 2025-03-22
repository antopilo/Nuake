#pragma once
#include "Nuake/Core/Core.h"

#include "EditorContext.h"

#include "Widgets/IEditorWidget.h"

#include <set>
#include <string>
#include <vector>
#include <concepts>

// Forward declarations
namespace Nuake
{
	class Scene;
	class Entity;
}

template <typename T>
concept DerivedFromEditorWidget = std::derived_from<T, IEditorWidget>;

class SceneEditorWindow
{
private:
	bool layoutInitialized;
	std::string windowID; // This is used for imgui docking

	EditorContext editorContext;

	ImGuiID dockId = 0;
	bool isFocused;
	std::vector<Scope<IEditorWidget>> widgets;

	// This is because imgui keeps cache of previously created dockspace, so we can reuse it.
	std::string imguiId;
	static std::set<std::string> previouslyCreatedEditors;

public:
	SceneEditorWindow(Ref<Nuake::Scene> scene);
	~SceneEditorWindow() = default;

public:
	void Save();
	void Update(float ts);
	void Draw();

	std::string GetWindowName() const;
	void DockTo(ImGuiID id)
	{
		dockId = id;
	}

	bool IsFocused() const;
	Ref<Nuake::Scene> GetScene() const;
	void SetScene(Ref<Nuake::Scene> scene);

private:
	template<DerivedFromEditorWidget T>
	inline void RegisterWidget()
	{
		widgets.push_back(CreateScope<T>(editorContext));
	}
};
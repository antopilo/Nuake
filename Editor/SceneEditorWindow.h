#pragma once
#include "src/Core/Core.h"

#include "IEditorWidget.h"
#include "EditorContext.h"

#include <string>
#include <vector>

// Forward declarations
namespace Nuake
{
	class Scene;
	class Entity;
}

class SceneHierarchyWidget;

class SceneEditorWindow
{
private:
	std::string windowID; // This is used for imgui docking

	EditorContext editorContext;

	std::vector<Scope<IEditorWidget>> widgets;
public:
	SceneEditorWindow(Ref<Nuake::Scene> scene);
	~SceneEditorWindow() = default;

public:
	void Update(float ts);
	void Draw();

private:
	template<class T>
	inline void RegisterWidget()
	{
		widgets.push_back(CreateScope<T>(editorContext));
	}

};
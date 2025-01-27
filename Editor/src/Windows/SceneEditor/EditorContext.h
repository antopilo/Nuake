#pragma once
#include "src/Core/Core.h"
#include "src/Core/MulticastDelegate.h"
#include "../../Actions/EditorSelection.h"


namespace Nuake
{
	class Scene;
}

class EditorContext
{
private:
	EditorSelection selection;
	Ref<Nuake::Scene> scene;
	std::string windowClass;

public:
	EditorContext() = default;
	~EditorContext() = default;

	EditorContext(Ref<Nuake::Scene> inScene, std::string inWindowClass) : 
		scene(inScene),
		windowClass(inWindowClass)
	{
	}

public:
	const std::string_view GetWindowClass() const { return windowClass; }
	MulticastDelegate<EditorSelection> OnSelectionChanged;

	const EditorSelection& GetSelection() const { return selection; }
	void SetSelection(EditorSelection inSelection)
	{
		selection = inSelection;
		OnSelectionChanged.Broadcast(selection);
	}

	Ref<Nuake::Scene> GetScene() const { return scene; }
};
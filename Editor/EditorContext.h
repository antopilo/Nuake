#pragma once
#include "src/Core/Core.h"
#include "src/Core/MulticastDelegate.h"

#include "src/Actions/EditorSelection.h"

namespace Nuake
{
	class Scene;
}

class EditorContext
{
private:
	EditorSelection selection;
	Ref<Nuake::Scene> scene;

public:
	EditorContext() = default;
	~EditorContext() = default;

	EditorContext(Ref<Nuake::Scene> inScene)
		: scene(inScene)
	{
	}

public:
	MulticastDelegate<EditorSelection> OnSelectionChanged;

	const EditorSelection& GetSelection() const { return selection; }
	void SetSelection(EditorSelection inSelection)
	{
		selection = selection;
		OnSelectionChanged.Broadcast(selection);
	}

	Ref<Nuake::Scene> GetScene() const { return scene; }
};
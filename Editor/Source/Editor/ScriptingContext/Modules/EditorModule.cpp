#include "EditorModule.h"
#include "../../Windows/EditorInterface.h"

void SelectEntity(int entityId)
{
	if (entityId == -1)
	{
		Nuake::EditorInterface::Selection = EditorSelection();
		return;
	}

	//Nuake::EditorInterface::Selection = EditorSelection(Nuake::Entity{ (entt::entity)entityId, Nuake::Engine::GetCurrentScene().get() });
}

int GetSelectedEntity()
{
	return -10;
}

void EditorNetAPI::RegisterMethods()
{
	RegisterMethod("Editor.SetSelectedEntityIcall", (void*)(&SelectEntity));
	RegisterMethod("Editor.GetSelectedEntityIcall", (void*)(&GetSelectedEntity));
}

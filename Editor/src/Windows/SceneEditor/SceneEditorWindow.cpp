#include "SceneEditorWindow.h"

#include "Widgets/SceneHierarchyWidget.h"
#include "Widgets/SelectionPropertyWidget.h"

#include <src/UI/ImUI.h>

using namespace Nuake;

SceneEditorWindow::SceneEditorWindow(Ref<Scene> inScene)
{
	editorContext = EditorContext(inScene, inScene->GetName());

	RegisterWidget<SceneHierarchyWidget>();
	RegisterWidget<SelectionPropertyWidget>();
}

void SceneEditorWindow::Update(float ts)
{
	for (auto& widget : widgets)
	{
		widget->Update(ts);
	}
}

void SceneEditorWindow::Draw()
{
	Ref<Scene> scene = editorContext.GetScene();
	const std::string sceneName = scene->GetName();

	// This is to prevent other windows of other scene editors to dock 
	ImGuiWindowClass windowClass;
	windowClass.ClassId = ImHashStr(editorContext.GetWindowClass().data());
	windowClass.DockingAllowUnclassed = false;
	ImGui::SetNextWindowClass(&windowClass);

	if (ImGui::Begin(sceneName.c_str()))
	{
		ImGuiWindowClass localSceneEditorClass;
		localSceneEditorClass.ClassId = ImHashStr(sceneName.c_str());
		std::string dockspaceId = std::string("Dockspace##" + sceneName);
		ImGui::DockSpace(ImGui::GetID(dockspaceId.c_str()), ImGui::GetContentRegionAvail(), ImGuiDockNodeFlags_None, &localSceneEditorClass);
		
		for (auto& widget : widgets)
		{
			widget->Draw();
		}

		ImGui::End();
	}
}


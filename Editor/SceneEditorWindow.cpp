#include "SceneEditorWindow.h"

#include "SelectionPropertyWidget.h"
#include "SceneHierarchyWidget.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

using namespace Nuake;

SceneEditorWindow::SceneEditorWindow(Ref<Scene> inScene)
{
	editorContext = EditorContext(inScene);

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
	// into this window
	ImGuiWindowClass windowClass;
	windowClass.ClassId = ImHashStr("SceneEditor");
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


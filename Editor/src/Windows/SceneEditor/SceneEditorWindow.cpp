#include "SceneEditorWindow.h"

#include "Widgets/SceneHierarchyWidget.h"
#include "Widgets/SelectionPropertyWidget.h"
#include "Widgets/LoggerWidget.h"
#include "Widgets/ViewportWidget.h"
#include "Widgets/FileBrowserWidget.h"

#include "src/Scene/Scene.h"

#include "src/UI/ImUI.h"

using namespace Nuake;

SceneEditorWindow::SceneEditorWindow(Ref<Scene> inScene) :
	editorContext(inScene, inScene->Path),
	layoutInitialized(false)
{
	RegisterWidget<SceneHierarchyWidget>();
	RegisterWidget<SelectionPropertyWidget>();
	RegisterWidget<LoggerWidget>();
	RegisterWidget<ViewportWidget>();
	RegisterWidget<FileBrowserWidget>();
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
	const std::string sceneName = scene->Path;

	// This is to prevent other windows of other scene editors to dock 
	ImGuiWindowClass windowClass;
	windowClass.ClassId = ImHashStr("SceneEditor");
	windowClass.DockingAllowUnclassed = false;
	ImGui::SetNextWindowClass(&windowClass);

	ImGui::SetNextWindowSizeConstraints({1280, 720}, { FLT_MAX, FLT_MAX });
	if (ImGui::Begin(std::string(ICON_FA_WINDOW_MAXIMIZE + std::string("  ") + sceneName).c_str()))
	{
		ImGuiWindowClass localSceneEditorClass;
		localSceneEditorClass.ClassId = ImHashStr(sceneName.c_str());
		std::string dockspaceName = std::string("Dockspace##" + sceneName);

		ImGuiID dockspaceId = ImGui::GetID(dockspaceName.c_str());
		ImGui::DockSpace(dockspaceId, ImGui::GetContentRegionAvail(), ImGuiDockNodeFlags_None, &localSceneEditorClass);

		for (auto& widget : widgets)
		{
			widget->Draw();
		}

		// Build initial docking layout
		if (!layoutInitialized)
		{
			auto dockbottomId = ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Down, 0.3f, nullptr, &dockspaceId);
			auto dockLeftId = ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Left, 0.3f, nullptr, &dockspaceId);
			auto dockRightId = ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Right, 0.5f, nullptr, &dockspaceId);

			widgets[0]->DockTo(dockLeftId);
			widgets[1]->DockTo(dockRightId);
			widgets[2]->DockTo(dockbottomId);
			widgets[3]->DockTo(dockspaceId);
			widgets[4]->DockTo(dockbottomId);
			layoutInitialized = true;
		}
	}
	ImGui::End();
}


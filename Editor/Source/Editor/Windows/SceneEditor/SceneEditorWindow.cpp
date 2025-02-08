#include "SceneEditorWindow.h"

#include "Widgets/SceneHierarchyWidget.h"
#include "Widgets/SelectionPropertyWidget.h"
#include "Widgets/LoggerWidget.h"
#include "Widgets/ViewportWidget.h"
#include "Widgets/FileBrowserWidget.h"

#include "Nuake/Scene/Scene.h"

#include "Nuake/UI/ImUI.h"
#include "../../Events/EditorRequests.h"

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

void SceneEditorWindow::Save()
{
	editorContext.GetScene()->Save();
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
	std::string sceneName = editorContext.GetScene()->Path.empty() ? "New Scene" : editorContext.GetScene()->Path;

	ImGuiID editorDockspaceId = ImGui::GetID("SceneEditorDockSpace");
	if (!layoutInitialized) 
	{
		
	}
	// This is to prevent other windows of other scene editors to dock 
	ImGuiWindowClass windowClass;
	windowClass.ClassId = ImHashStr("SceneEditor");
	windowClass.DockingAllowUnclassed = false;
	ImGui::SetNextWindowClass(&windowClass);
	ImGui::SetNextWindowDockID(ImGui::GetID("SceneEditorDockSpace"));
	ImGui::SetNextWindowSizeConstraints({1280, 720}, { FLT_MAX, FLT_MAX });
	bool shouldStayOpen = true;
	std::string windowName = std::string(ICON_FA_WINDOW_MAXIMIZE + std::string("   ") + sceneName);
	
	if (ImGui::Begin(windowName.c_str(), &shouldStayOpen))
	{
		this->isFocused = true;

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
		}
	}
	else
	{
		this->isFocused = false;
	}
	ImGui::End();
	if (!layoutInitialized)
	{
		//ImGui::DockBuilderSplitNode(dockId, ImGuiDir_Down, 0.3f, nullptr, &dockId);
		//ImGui::DockBuilderDockWindow(windowName.c_str(), dockId);

		if (dockId != 0)
		{
			//ImGui::DockBuilderDockWindow(windowName.c_str(), dockId);
		}
		layoutInitialized = true;
	}

	if (!shouldStayOpen)
	{
		EditorRequests::Get().RequestCloseEditorWindow(editorContext.GetScene()->Path);
	}
}

std::string SceneEditorWindow::GetWindowName() const
{
	return editorContext.GetScene()->Path;
}

bool SceneEditorWindow::IsFocused() const
{
	return this->isFocused;
}

void SceneEditorWindow::SetScene(Ref<Scene> scene)
{
	editorContext.SetScene(scene);

	for (auto& widget : widgets)
	{
		widget->OnSceneChanged(scene);
	}
}

Ref<Scene> SceneEditorWindow::GetScene() const
{
	return this->editorContext.GetScene();
}
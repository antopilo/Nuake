#pragma once
#include <Nuake/Scene/Entities/Entity.h>

#include "imgui/imgui.h"

#include <imgui/ImGuizmo.h>
#include "Nuake/FileSystem/FileSystem.h"

#include "../Actions/EditorSelection.h"
#include "EditorSelectionPanel.h"
#include "WelcomeWindow.h"
#include "NewProjectWindow.h"
#include "AudioWindow.h"
#include "../Windows/TrenchbroomConfiguratorWindow.h"
#include "../Commands/CommandBuffer.h"
#include "../Commands/ICommand.h"
#include "MapImporterWindow.h"

#include <Nuake/Scripting/ScriptingEngineNet.h>
#include "ProjectSettings/ProjectSettingsWindow.h"
#include "PrefabEditor/PrefabEditorWindow.h"

#include "SceneEditor/SceneEditorWindow.h"

using namespace NuakeEditor;

namespace Nuake
{
	class Material;
	class FileSystemUI;
	class VulkanImage;
	class DebugCmd;

	class EditorInterface
	{
	private:
		bool m_TitleBarHovered = false;
		std::vector<CompilationError> errors;
		std::vector<Ref<PrefabEditorWindow>> prefabEditors;
		std::vector<Ref<SceneEditorWindow>> sceneEditors;

		Ref<Scene> SceneSnapshot;
		static NuakeEditor::CommandBuffer* mCommandBuffer;

		std::string searchQuery = "";
		bool isControllingCamera = false;
		bool m_DrawGrid = false;
		bool m_DrawAxis = true;
		bool m_ShowImGuiDemo = false;
		bool m_DrawShapes = true;
		bool m_DrawGizmos = true;
		bool m_DrawCamPreview = false;
		bool m_DrawNavMesh = true;
		bool m_ShowOverlay = true;
		bool m_IsHoveringViewport = false;
		bool m_IsViewportFocused = false;
		bool m_IsRenaming = false;
		bool m_ShouldUnfoldEntityTree = false;
		bool m_ShowTrenchbroomConfigurator = false;
		bool m_ShowGpuResources = false;
		bool m_ShowProjectSettings = false;
		bool m_ShowMapImporter = false;
		Vector2 m_ViewportPos = {0, 0};
		Vector2 m_ViewportSize = {};

		ImGuizmo::OPERATION CurrentOperation = ImGuizmo::TRANSLATE;
		ImGuizmo::MODE CurrentMode = ImGuizmo::WORLD;
		bool UseSnapping = true;
		Vector3 CurrentSnapping = { 0.0f, 0.0f, 0.0f };
		Ref<Material> m_SelectedMaterial;
		Ref<Directory> m_CurrentDirectory;

		// Filebrowser
		bool showFloatingFileBrowser;
		bool showFloatingLogger;

		bool m_IsMaterialSelected = false;

		std::string m_StatusMessage = "";
		Color m_StatusBarColor = Color(0.08f, 0.08f, 0.08f, 1.0f);
	public:
		WelcomeWindow* _WelcomeWindow;
		NewProjectWindow* _NewProjectWindow;
		static bool isCreatingNewProject;

		AudioWindow* _audioWindow;
		FileSystemUI* filesystem;

		//Scope<FileSystemUI> floatingFileBrowser;
		bool isNewProject = false;
		static EditorSelection Selection;
		EditorSelectionPanel* SelectionPanel;
		TrenchbroomConfiguratorWindow m_TrenchhbroomConfigurator;
		MapImporterWindow m_MapImporter;

		ProjectSettingsWindow* m_ProjectSettingsWindow;
		Ref<VulkanImage> NuakeTexture;
		Ref<VulkanImage> CloseIconTexture;
		Ref<VulkanImage> MaximizeTexture;
		Ref<VulkanImage> RestoreTexture;
		Ref<VulkanImage> MinimizeTexture;
		EditorInterface(CommandBuffer& commandBuffer);

		static ImFont* bigIconFont;
		void BuildFonts();
		void DrawTitlebar(float& outHeight);
		void Init();
		
		void Draw();
		void Update(float ts);
		void DrawMenuBar();
		bool BeginMenubar(const ImRect& barRectangle);
		void EndMenubar();

		void OnSceneLoaded(Ref<Scene> scene);
		void OnDebugDraw(Nuake::DebugCmd& cmd);

		void SetStatusMessage(const std::string& msg, const Color& color = Color(0.08f, 0.08f, 0.08f, 1.0f)) { m_StatusMessage = msg; m_StatusBarColor = color; }
		void DrawViewport();
		void DrawStatusBar();
		void DrawMenuBars();
		void DrawEntityTree(Entity ent, bool drawChildrens = true);
		void DrawSceneTree();
		void DrawLogger();
		void DrawProjectSettings();
		void Overlay();

		// Signals
		void OnRequestLoadScene(Ref<File> file);
		void OnRequestCloseEditorWindow(std::string windowName);

		void OpenPrefabWindow(const std::string& prefabPath);
		void OpenSceneWindow(const std::string& scenePath);

		bool ShouldDrawAxis() const { return m_DrawAxis; }
		bool ShouldDrawShapes() const { return m_DrawShapes; }
		bool ShouldDrawGizmos() const { return m_DrawGizmos; }
		bool ShouldDrawNavMesh() const { return m_DrawNavMesh; }
		bool LoadProject(const std::string& projectPath);

	public:
		std::string GetEntityTypeName(const Entity& entity) const;
		static void PushCommand(ICommand&& command);

	public:
		void OnWindowFocused();
		void OnDragNDrop(const std::vector<std::string>& paths);
	};
}

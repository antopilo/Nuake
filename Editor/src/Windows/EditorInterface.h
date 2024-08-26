#pragma once
#include <src/Scene/Entities/Entity.h>

#include "src/Vendors/imgui/imgui.h"

#include <src/Vendors/imgui/ImGuizmo.h>
#include "src/Core/FileSystem.h"

#include "../Actions/EditorSelection.h"
#include "EditorSelectionPanel.h"
#include "WelcomeWindow.h"
#include "AudioWindow.h"
#include "../Windows/TrenchbroomConfiguratorWindow.h"
#include "../Commands/CommandBuffer.h"
#include "../Commands/ICommand.h"
#include "MapImporterWindow.h"

#include <src/Scripting/ScriptingEngineNet.h>
#include "ProjectSettings/ProjectSettingsWindow.h"

using namespace NuakeEditor;

namespace Nuake
{
	class Material;
	class FileSystemUI;

	class EditorInterface
	{
	private:
		std::vector<CompilationError> errors;
		Ref<Scene> SceneSnapshot;
		static NuakeEditor::CommandBuffer* mCommandBuffer;

		bool m_DrawGrid = false;
		bool m_DrawAxis = true;
		bool m_ShowImGuiDemo = false;
		bool m_DebugCollisions = true;
		bool m_DrawNavMesh = true;
		bool m_ShowOverlay = true;
		bool m_IsHoveringViewport = false;
		bool m_IsViewportFocused = false;
		bool m_IsRenaming = false;
		bool m_ShouldUnfoldEntityTree = false;
		bool m_ShowTrenchbroomConfigurator = false;
		bool m_ShowProjectSettings = false;
		bool m_ShowMapImporter = false;

		Vector2 m_ViewportPos = {0, 0};
		Vector2 m_ViewportSize = {};

		ImGuizmo::OPERATION CurrentOperation = ImGuizmo::TRANSLATE;
		ImGuizmo::MODE CurrentMode = ImGuizmo::WORLD;
		Ref<Material> m_SelectedMaterial;
		Ref<Directory> m_CurrentDirectory;

		bool m_IsMaterialSelected = false;

		std::string m_StatusMessage = "";
		Color m_StatusBarColor = Color(0.08f, 0.08f, 0.08f, 1.0f);
	public:
		WelcomeWindow* _WelcomeWindow;
		AudioWindow* _audioWindow;
		FileSystemUI* filesystem;
		static EditorSelection Selection;
		EditorSelectionPanel SelectionPanel;
		TrenchbroomConfiguratorWindow m_TrenchhbroomConfigurator;
		MapImporterWindow m_MapImporter;

		ProjectSettingsWindow* m_ProjectSettingsWindow;

		EditorInterface(CommandBuffer& commandBuffer);

		static ImFont* bigIconFont;
		void BuildFonts();
		void Init();
		void Draw();
		void Update(float ts);
		void DrawMenuBar();
		void SetStatusMessage(const std::string& msg, const Color& color = Color(0.08f, 0.08f, 0.08f, 1.0f)) { m_StatusMessage = msg; m_StatusBarColor = color; }
		void DrawViewport();
		void DrawStatusBar();
		void DrawMenuBars();
		void DrawEntityTree(Entity ent);
		void DrawSceneTree();
		void DrawLogger();
		void DrawProjectSettings();
		bool EntityContainsItself(Entity ent1, Entity ent2);
		void Overlay();

		bool ShouldDrawAxis() const { return m_DrawAxis; }
		bool ShouldDrawCollision() const { return m_DebugCollisions; }
		bool ShouldDrawNavMesh() const { return m_DrawNavMesh; }
		bool LoadProject(const std::string& projectPath);

	public:
		std::string GetEntityTypeName(const Entity& entity) const;
		static void PushCommand(ICommand&& command);
	};
}

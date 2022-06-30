#pragma once
#include <src/Scene/Entities/Entity.h>
#include "src/Vendors/imgui/imgui.h"
#include <src/Vendors/imgui/ImGuizmo.h>
#include "src/Core/FileSystem.h"

#include "../Actions/EditorSelection.h"
#include "EditorSelectionPanel.h"
#include "WelcomeWindow.h"
namespace Nuake {
	class Material;
	class FileSystemUI;
	class EditorInterface
	{
	private:
		bool m_DrawGrid = false;
		bool m_ShowImGuiDemo = false;
		bool m_DebugCollisions = false;
		bool m_ShowOverlay = true;
		ImGuizmo::OPERATION CurrentOperation = ImGuizmo::TRANSLATE;
		ImGuizmo::MODE CurrentMode = ImGuizmo::WORLD;
		Ref<Material> m_SelectedMaterial;
		Ref<Directory> m_CurrentDirectory;

		bool m_IsMaterialSelected = false;
	public:
		WelcomeWindow* _WelcomeWindow;
		FileSystemUI* filesystem;
		EditorSelection Selection;
		EditorSelectionPanel SelectionPanel;

		EditorInterface();

		static ImFont* bigIconFont;
		void BuildFonts();
		void Init();
		void Draw();
		void DrawViewport();
		void DrawEntityTree(Entity ent);
		void DrawSceneTree();
		void DrawLogger();
		bool EntityContainsItself(Entity ent1, Entity ent2);
		void Overlay();
	};
}

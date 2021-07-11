#pragma once
#include <src/Scene/Entities/Entity.h>
#include "src/Vendors/imgui/imgui.h"
#include <src/Vendors/imgui/ImGuizmo.h>
#include "src/Core/FileSystem.h"
#include "FileSystemUI.h"



namespace Nuake {
	class Material;
	class EditorInterface
	{
	private:
		FileSystemUI filesystem = FileSystemUI();
		Entity m_SelectedEntity;
		bool m_IsEntitySelected = false;
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
		static ImFont* bigIconFont;
		void BuildFonts();
		void Init();
		void Draw();
		void DrawViewport();
		void DrawEntityTree(Entity ent);
		void DrawSceneTree();
		void DrawEntityPropreties();
		void DrawGizmos();
		void DrawFileSystem();
		void DrawDirectoryExplorer();
		void DrawLogger();
		void DrawDirectory(Ref<Directory> directory);
		bool EntityContainsItself(Entity ent1, Entity ent2);
		void DrawFile(Ref<File> file);
		void DrawRessourceWindow();
		void DrawInit();
		void EditorInterfaceDrawFiletree(Ref<Directory> dir);
		void Overlay();
	};
}

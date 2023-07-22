#pragma once
#include <src/Core/FileSystem.h>
#include <src/Scene/Entities/Entity.h>
#include "EditorInterface.h"

namespace Nuake {
	class FileSystemUI
	{
	private:
		EditorInterface* Editor;

	public:
		static Ref<Directory> m_CurrentDirectory;
		bool m_hasClickedOnFile;

		FileSystemUI(EditorInterface* editor)
		{
			m_CurrentDirectory = FileSystem::RootDirectory;
			m_hasClickedOnFile = false;
			Editor = editor;
		}

		void Draw();
		void DrawDirectoryContent();
		void DrawFiletree();
		void EditorInterfaceDrawFiletree(Ref<Directory> dir);
		void DrawDirectory(Ref<Directory> directory);
		bool EntityContainsItself(Entity source, Entity target);
		void DrawFile(Ref<File> file, uint32_t drawId);
		void DrawDirectoryExplorer();
		bool DeletePopup();
		void DrawContextMenu();
		void RefreshFileBrowser();
	};
}

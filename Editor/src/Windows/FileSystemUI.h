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
		Ref<Directory> m_CurrentDirectory;

		FileSystemUI(EditorInterface* editor) {
			m_CurrentDirectory = FileSystem::RootDirectory;
			Editor = editor;
		}

		void Draw();
		void DrawDirectoryContent();
		void DrawFiletree();
		void EditorInterfaceDrawFiletree(Ref<Directory> dir);
		void DrawDirectory(Ref<Directory> directory);
		bool EntityContainsItself(Entity source, Entity target);
		void DrawFile(Ref<File> file);
		void DrawDirectoryExplorer();
		void DrawContextMenu();
	};
}

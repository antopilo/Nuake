#pragma once
#include <src/Core/FileSystem.h>
#include <src/Scene/Entities/Entity.h>

namespace Nuake {
	class FileSystemUI
	{
	private:


	public:
		Ref<Directory> m_CurrentDirectory;
		FileSystemUI() {
			m_CurrentDirectory = FileSystem::RootDirectory;
		}
		void Draw();
		void DrawDirectoryContent();
		void DrawFiletree();
		void EditorInterfaceDrawFiletree(Ref<Directory> dir);
		void DrawDirectory(Ref<Directory> directory);
		bool EntityContainsItself(Entity source, Entity target);
		void DrawFile(Ref<File> file);
		void DrawDirectoryExplorer();
	};
}

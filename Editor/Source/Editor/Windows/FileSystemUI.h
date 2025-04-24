#pragma once
#include "Nuake/FileSystem/FileSystem.h"
#include <Nuake/Scene/Entities/Entity.h>
#include "EditorInterface.h"

namespace Nuake {
	class FileSystemUI
	{
	private:
		EditorInterface* Editor;
		const int maxTextureLoadedPerFrame = 4;
		int textureLoadedThisFrame = 0;

	public:
		static Ref<Directory> m_CurrentDirectory;
		bool m_HasClickedOnFile;
		std::string m_SearchKeyword;

		FileSystemUI(EditorInterface* editor)
		{
			m_CurrentDirectory = FileSystem::RootDirectory;
			m_HasClickedOnFile = false;
			Editor = editor;
		}

		void Draw();
		void DrawDirectoryContent();
		void DrawFiletree();
		void EditorInterfaceDrawFiletree(Ref<Directory> dir);
		void DrawDirectory(Ref<Directory> directory, uint32_t drawId);
		bool EntityContainsItself(Entity source, Entity target);
		void DrawFile(Ref<File> file, uint32_t drawId);
		void DrawDirectoryExplorer();
		bool DeletePopup();
		void DrawContextMenu();
		void RefreshFileBrowser();

		Color GetColorByFileType(FileType fileType);

		void Scan();
	};
}

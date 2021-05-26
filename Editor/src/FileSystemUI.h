#pragma once

#include <src/Core/FileSystem.h>

#include <src/Scene/Entities/Entity.h>
class FileSystemUI
{
private:
	Ref<Directory> m_CurrentDirectory;

public:
	void Draw();
	void DrawDirectoryContent();
	void DrawFiletree();
	void EditorInterfaceDrawFiletree(Ref<Directory> dir);
	void DrawDirectory(Ref<Directory> directory);
	bool EntityContainsItself(Entity source, Entity target);
	void DrawFile(Ref<File> file);
	void DrawDirectoryExplorer();
};
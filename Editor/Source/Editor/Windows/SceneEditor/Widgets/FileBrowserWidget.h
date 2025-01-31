#pragma once

#include "Nuake/Core/Core.h"
#include "Nuake/Core/Maths.h"
#include "Nuake/FileSystem/FileTypes.h"

#include "IEditorWidget.h"

namespace Nuake
{
	class Directory;
	class File;
}

class FileBrowserWidget : public IEditorWidget
{
private:
	float splitterSizeLeft = 300.0f;
	float splitterSizeRight = 300.0f;

	Ref<Nuake::Directory> currentDirectory;

	std::string searchQuery;

public:
	FileBrowserWidget(EditorContext& inCtx);
	~FileBrowserWidget() = default;

public:
	void Update(float ts) override;
	void Draw() override;

	void DrawFiletree(Ref<Nuake::Directory> dir);
	void DrawDirectory(Ref<Nuake::Directory> dir, uint32_t drawId);
	void DrawFile(Ref<Nuake::File> file, uint32_t drawId);

    Nuake::Color GetColorByFileType(Nuake::FileType fileType);
    
};
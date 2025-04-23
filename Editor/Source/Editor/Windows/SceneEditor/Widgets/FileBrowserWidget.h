#pragma once

#include "Nuake/Core/Core.h"
#include "Nuake/Core/Maths.h"
#include "Nuake/FileSystem/FileTypes.h"

#include "IEditorWidget.h"

#include "../../../../../AnimatedValue.h"

namespace Nuake
{
	class Directory;
	class File;
}

class FileBrowserWidget : public IEditorWidget
{
private:
	AnimatedValue<float> opacity;
	float splitterSizeLeft = 300.0f;
	float splitterSizeRight = 300.0f;
	int maxImageLoaded = 2;
	int ImageLoaded = 0;
	Ref<Nuake::Directory> currentDirectory;
	Ref<Nuake::Directory> queueDirectory;

	std::string searchQuery;

public:
	FileBrowserWidget(EditorContext& inCtx);
	~FileBrowserWidget() = default;

public:
	void SetCurrentDirectory(Ref<Nuake::Directory> dir);
	void Update(float ts) override;
	void Draw() override;

	void DrawFiletree(Ref<Nuake::Directory> dir);
	void DrawDirectory(Ref<Nuake::Directory> dir, uint32_t drawId);
	void DrawFile(Ref<Nuake::File> file, uint32_t drawId);

    Nuake::Color GetColorByFileType(Nuake::FileType fileType);
    
};
#pragma once
#include "IEditorWidget.h"

#include "Nuake/Scene/Entities/Entity.h"

class EditorContext;

class SceneHierarchyWidget : public IEditorWidget
{
public:
	SceneHierarchyWidget(EditorContext& inCtx);
	~SceneHierarchyWidget() = default;

public:
	void Update(float ts) override;
	void Draw() override;

private:
	std::string searchQuery;
	bool isRenaming;
	Nuake::Entity deletionQueue;

	void DrawSearchBar();
	void DrawCreateEntityButton();

	void DrawEntityTree();
	void DrawEntity(Nuake::Entity entity, bool drawChildrens = true);
};

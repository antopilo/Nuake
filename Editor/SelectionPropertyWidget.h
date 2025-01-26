#pragma once
#include "IEditorWidget.h"

class EditorContext;

class SelectionPropertyWidget : public IEditorWidget
{
public:
	SelectionPropertyWidget(EditorContext& inCtx);
	~SelectionPropertyWidget() = default;

public:
	void Update(float ts) override;
	void Draw() override;

private:
	
};
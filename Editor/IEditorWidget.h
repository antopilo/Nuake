#pragma once

class EditorContext;

class IEditorWidget
{
protected:
	EditorContext& editorContext;

public:
	IEditorWidget(EditorContext& inContext) : editorContext(inContext) {}
	~IEditorWidget() {};

public:
	virtual void Update(float ts) = 0;
	virtual void Draw() = 0;
};
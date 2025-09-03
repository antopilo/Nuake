#pragma once

#include "IEditorWidget.h"

class LoggerWidget : public IEditorWidget
{
private:
	bool LogErrors = true;
	bool LogWarnings = true;
	bool LogDebug = true;
	bool AutoScroll = true;

public:
	LoggerWidget(EditorContext& inCtx) : IEditorWidget(inCtx) {}
	~LoggerWidget() = default;

public:
	void Update(float ts) override;
	void Draw() override;
};
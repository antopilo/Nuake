#pragma once

#include "IEditorWidget.h"

#include <imgui/ImGuizmo.h>

class EditorContext;

class ViewportWidget : public IEditorWidget
{
private:
	ImGuizmo::OPERATION CurrentOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE CurrentMode = ImGuizmo::WORLD;
	bool UseSnapping = true;
	Nuake::Vector3 CurrentSnapping = { 0.05f, 0.05f, 0.05f };

public:
	ViewportWidget(EditorContext& context) : IEditorWidget(context) {}
	~ViewportWidget() = default;

public:
	void Update(float ts) override;
	void Draw() override;
};
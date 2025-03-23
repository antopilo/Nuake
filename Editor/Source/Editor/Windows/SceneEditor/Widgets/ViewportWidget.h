#pragma once

#include "IEditorWidget.h"

#include <imgui/ImGuizmo.h>

class EditorContext;

namespace Nuake
{
	class Viewport;
	class DebugCmd;
}

class ViewportWidget : public IEditorWidget
{
private:
	Ref<Nuake::Viewport> sceneViewport;

	ImGuizmo::OPERATION CurrentOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE CurrentMode = ImGuizmo::WORLD;
	bool UseSnapping = true;
	Nuake::Vector3 CurrentSnapping = { 0.05f, 0.05f, 0.05f };

	bool isHoveringViewport;
public:
	ViewportWidget(EditorContext& context);
	~ViewportWidget();

public:
	void Update(float ts) override;
	void Draw() override;
	void OnSceneChanged(Ref<Nuake::Scene> scene) override;

	void OnDebugDraw(Nuake::DebugCmd& debugCmd);
};
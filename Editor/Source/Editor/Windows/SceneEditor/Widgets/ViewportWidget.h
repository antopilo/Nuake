#pragma once

#include "IEditorWidget.h"

#include <imgui/ImGuizmo.h>
#include "../../../../../AnimatedValue.h"

class EditorContext;

namespace Nuake
{
	class Viewport;
	class DebugCmd;
	class DebugLineCmd;
}

class ViewportWidget : public IEditorWidget
{
private:
	enum class GizmoDrawingModes : char16_t
	{
		EditorOnly,
		Always,
		None
	};

private:
	Ref<Nuake::Viewport> sceneViewport;

	AnimatedValue<float> outlineSize;
	AnimatedValue<float> overlayOpacity;

	// Gizmo
	GizmoDrawingModes gizmoDrawingMode;

	ImGuizmo::OPERATION CurrentOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE CurrentMode = ImGuizmo::WORLD;
	bool UseSnapping = true;
	Nuake::Vector3 CurrentSnapping = { 0.05f, 0.05f, 0.05f };

	bool IsControllingCamera = false;
	bool isHoveringViewport;

public:
	ViewportWidget(EditorContext& context);
	~ViewportWidget();

public:
	void Update(float ts) override;
	void Draw() override;
	void OnSceneChanged(Ref<Nuake::Scene> scene) override;

	void OnLineDraw(Nuake::DebugLineCmd& lineCmd);
	void OnDebugDraw(Nuake::DebugCmd& debugCmd);

private:
	void DrawOverlay();
};
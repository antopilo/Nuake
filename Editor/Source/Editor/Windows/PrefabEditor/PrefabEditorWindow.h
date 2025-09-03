#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Core/Maths.h"

#include "../EditorSelectionPanel.h"
#include <imgui/ImGuizmo.h>

namespace Nuake
{
	class Prefab;
	class Scene;
	class FrameBuffer;
	class Texture;
	class Entity;
}

class PrefabEditorWindow
{
public:
	PrefabEditorWindow(Ref<Nuake::Prefab> prefab);
	~PrefabEditorWindow() {
		ASSERT(false);
	};

	void Update(float ts);
	void Draw();

private:
	Ref<Nuake::Prefab> prefab;
	Ref<Nuake::Scene> virtualScene;
	Ref<Nuake::FrameBuffer> viewportFramebuffer;
	Ref<Nuake::Texture> viewportTexture;

	bool isViewportFocused;
	bool isHoveringViewport;
	bool isRenaming;
	bool isInitialized = false;

	Scope<EditorSelectionPanel> SelectionPanel;
	EditorSelection Selection;
	Nuake::Entity QueueDeletion;
	ImGuizmo::OPERATION CurrentOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE CurrentMode = ImGuizmo::WORLD;
	bool UseSnapping = true;
	Nuake::Vector3 CurrentSnapping = Nuake::Vector3{ 0.0f, 0.0f, 0.0f };
	bool isControllingCamera = false;
private:
	void DrawViewportWindow();
	void DrawOverlay();
	void RenderScene();
	void DrawEntityTree(Nuake::Entity e);
	void Save();
};
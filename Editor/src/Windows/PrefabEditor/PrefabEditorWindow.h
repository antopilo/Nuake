#pragma once
#include "src/Core/Core.h"

#include "../EditorSelectionPanel.h"


namespace Nuake
{
	class Prefab;
	class Scene;
	class FrameBuffer;
	class Texture;
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

	EditorSelectionPanel SelectionPanel;
	EditorSelection Selection;

private:
	void DrawViewportWindow();
	void RenderScene();
	void DrawEntityTree(Nuake::Entity e);
	void Save();
};
#pragma once
#include "../Actions/EditorSelection.h"
#include "src/Scene/Entities/Entity.h"
#include "src/Core/FileSystem.h"

#include "../ComponentsPanel/TransformPanel.h"
#include "../ComponentsPanel/LightPanel.h"
#include "../ComponentsPanel/ScriptPanel.h"
#include "../ComponentsPanel/MeshPanel.h"
#include "../ComponentsPanel/QuakeMapPanel.h"

class EditorSelectionPanel {
private:
	TransformPanel mTransformPanel;
	LightPanel mLightPanel;
	ScriptPanel mScriptPanel;
	MeshPanel mMeshPanel;
	QuakeMapPanel mQuakeMapPanel;
public:
	EditorSelectionPanel();

	void Draw(EditorSelection selection);

	void DrawNone();
	void DrawEntity(Nuake::Entity entity);
	void DrawAddComponentMenu(Nuake::Entity entity);

	void DrawFile(Nuake::File* file);
	void DrawResource(Nuake::Resource resource);
};
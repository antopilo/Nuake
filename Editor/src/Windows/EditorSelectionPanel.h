#pragma once
#include "../Actions/EditorSelection.h"
#include "src/Scene/Entities/Entity.h"

#include "src/Core/FileSystem.h"
#include <src/Resource/Project.h>

#include "../ComponentsPanel/TransformPanel.h"
#include "../ComponentsPanel/LightPanel.h"
#include "../ComponentsPanel/ScriptPanel.h"
#include "../ComponentsPanel/MeshPanel.h"
#include "../ComponentsPanel/QuakeMapPanel.h"
#include "../ComponentsPanel/CameraPanel.h"
#include "../ComponentsPanel/RigidbodyPanel.h"
#include "../ComponentsPanel/BoxColliderPanel.h"
#include "../ComponentsPanel/CapsuleColliderPanel.h"
#include "../ComponentsPanel/CylinderColliderPanel.h"
#include "../ComponentsPanel/SphereColliderPanel.h"
#include "../ComponentsPanel/MeshColliderPanel.h"
#include "../ComponentsPanel/CharacterControllerPanel.h"

class EditorSelectionPanel {
private:
	TransformPanel mTransformPanel;
	LightPanel mLightPanel;
	ScriptPanel mScriptPanel;
	MeshPanel mMeshPanel;
	QuakeMapPanel mQuakeMapPanel;
	CameraPanel mCameraPanel;
	RigidbodyPanel mRigidbodyPanel;
	BoxColliderPanel mBoxColliderPanel;
	SphereColliderPanel mSphereColliderPanel;
	MeshColliderPanel mMeshColliderPanel;
	CapsuleColliderPanel mCapsuleColliderPanel;
	CylinderColliderPanel mCylinderColliderPanel;

	CharacterControllerPanel mCharacterControllerPanel;

	Ref<Nuake::File> currentFile;
	Ref<Nuake::Resource> selectedResource;

public:
	EditorSelectionPanel();

	void Draw(EditorSelection selection);

	void DrawNone();
	void DrawEntity(Nuake::Entity entity);
	void DrawAddComponentMenu(Nuake::Entity entity);

	void DrawFile(Nuake::File* file);
	void DrawResource(Nuake::Resource resource);

private:
	void ResolveFile(Ref<Nuake::File> file);
	void DrawMaterialPanel(Ref<Nuake::Material> material);
	void DrawProjectPanel(Ref<Nuake::Project> project);
};
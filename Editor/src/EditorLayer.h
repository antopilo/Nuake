#pragma once
#include <src/Application/Layer.h>
#include "Commands/CommandBuffer.h"
#include "src/AI/NavMeshDebugDrawer.h"

namespace Nuake {

	class EditorInterface;

}

class GizmoDrawer;

using namespace NuakeEditor;

class EditorLayer : public Nuake::Layer
{
public:
	EditorLayer() : mCommandBuffer() {}

	virtual void OnAttach() override;
	virtual void OnUpdate() override;
	virtual void OnDetach() override;

	virtual void OnWindowFocused() override;

private:
	CommandBuffer mCommandBuffer;
	Nuake::EditorInterface* m_EditorInterface;
	GizmoDrawer* m_GizmoDrawer;

	Nuake::NavMeshDebugDrawer m_NavMeshDrawer;
};
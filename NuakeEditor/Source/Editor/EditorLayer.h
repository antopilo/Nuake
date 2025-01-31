#pragma once
#include <Nuake/Application/Layer.h>
#include "Commands/CommandBuffer.h"
#include "Nuake/AI/NavMeshDebugDrawer.h"

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
	virtual void OnDragNDrop(const std::vector<std::string>& paths) override;

private:
	CommandBuffer mCommandBuffer;
	Nuake::EditorInterface* m_EditorInterface;
	GizmoDrawer* m_GizmoDrawer;

	Nuake::NavMeshDebugDrawer m_NavMeshDrawer;
};
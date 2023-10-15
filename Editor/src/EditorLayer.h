#pragma once
#include <src/Application/Layer.h>

namespace Nuake {

	class EditorInterface;

}

class GizmoDrawer;


class EditorLayer : public Nuake::Layer
{
public:
	virtual void OnAttach() override;
	virtual void OnUpdate() override;
	virtual void OnDetach() override;

private:
	Nuake::EditorInterface* m_EditorInterface;
	GizmoDrawer* m_GizmoDrawer;
};
#pragma once
#include <src/Application/Application.h>
#include <src/Window.h>

#include "Windows/EditorInterface.h"
#include "Misc/GizmoDrawer.h"

#include "Commands/CommandBuffer.h"
#include "../LaunchSettings.h"

using namespace NuakeEditor;

class DragNDropModule : public Nuake::IApplicationModule
{
public:
	 void OnInit() override 
	 {
		
	 }
};

class EditorApplication : public Nuake::Application
{
private:
	CommandBuffer mCommandBuffer;

public:
	EditorApplication(const Nuake::ApplicationSpecification& specification, const LaunchSettings& launchSettings)
		: Application(specification),
		m_Editor(nullptr),
		m_LaunchSettings(launchSettings)
	{
	}

	virtual void OnInit() override;

	// virtual void OnUpdate() override;
	virtual void OnShutdown() override;

private:
	Nuake::EditorInterface* m_Editor;
	LaunchSettings m_LaunchSettings;
};
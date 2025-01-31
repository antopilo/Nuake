#pragma once

#include "../EditorContext.h"

#include "Nuake/UI/ImUI.h"

class IEditorWidget
{
protected:
	EditorContext& editorContext;

private:
	std::string widgetName;

public:
	IEditorWidget(EditorContext& inContext) : editorContext(inContext) {}
	virtual ~IEditorWidget() {};

public:
	virtual void Update(float ts) = 0;
	virtual void Draw() = 0;

	void DockTo(uint32_t dockId)
	{
		ImGui::DockBuilderDockWindow(widgetName.c_str(), dockId);
	}

	bool BeginWidgetWindow(const std::string_view& name)
	{
		return BeginWidgetWindow(name.data());
	}

	bool BeginWidgetWindow(const char* name)
	{
		ImGuiWindowClass windowClass;
		windowClass.ClassId = ImHashStr(editorContext.GetWindowClass().data());
		windowClass.DockingAllowUnclassed = false;
		ImGui::SetNextWindowClass(&windowClass);

		widgetName = std::string(name) + "##" + editorContext.GetScene()->Path;
		return ImGui::Begin(widgetName.c_str());
	}
};
#pragma once

#include "../EditorContext.h"

#include "src/UI/ImUI.h"

class IEditorWidget
{
protected:
	EditorContext& editorContext;

public:
	IEditorWidget(EditorContext& inContext) : editorContext(inContext) {}
	virtual ~IEditorWidget() {};

public:
	virtual void Update(float ts) = 0;
	virtual void Draw() = 0;

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

		std::string nameStr = std::string(name) + "##" + editorContext.GetScene()->GetName();
		return ImGui::Begin(nameStr.c_str());
	}
};
#include "SelectionPropertyWidget.h"

#include "src/UI/ImUI.h"

SelectionPropertyWidget::SelectionPropertyWidget(EditorContext& inCtx) 
	: IEditorWidget(inCtx)
{

}

void SelectionPropertyWidget::Update(float ts)
{
	if (BeginWidgetWindow("Selection Properties"))
	{
		ImGui::Text("Selection Properties");
		ImGui::End();
	}
}

void SelectionPropertyWidget::Draw()
{
	
}
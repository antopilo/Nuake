#include "UIComponent.h"

using namespace Nuake;

void UIComponent::SetUIFile(ResourceFile file)
{
	UIFilePath = file;
}

ResourceFile UIComponent::GetUIFile()
{
	return UIFilePath;
}

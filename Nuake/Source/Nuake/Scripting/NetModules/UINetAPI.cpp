#include "UINetAPI.h"
#include "Nuake/Core/Core.h"
#include "Nuake/Resource/ResourceManager.h"
#include "Nuake/Resource/UI.h"
#include "Nuake/Resource/UUID.h"
#include "Nuake/UI/Nodes/Canvas.h"
#include "Nuake/UI/Nodes/Text.h"

#include <Coral/String.hpp>

#include <iostream>


using namespace Nuake;

UUID UUIDFromString(const std::string& input)
{
	return std::stoull(input);
}

Coral::String FindChildByIDIcall(const Coral::String& canvasUUID, const Coral::String& id, const Coral::String& id2)
{
	if (!ResourceManager::IsResourceLoaded(UUIDFromString(canvasUUID)))
	{
		Logger::Log("Error finding child, canvas is not loaded.", ".net/ui", CRITICAL);
		return Coral::String::New(std::to_string(0));
	}

	Ref<UIResource> uiResource = ResourceManager::GetResource<UIResource>(UUIDFromString(canvasUUID));
	Ref<Canvas> canvas = uiResource->GetCanvas();
	if(Ref<Node> searchNode = canvas->GetNodeByUUID(UUIDFromString(id));
		searchNode != nullptr)
	{
		Ref<Node> foundNode;
		if (searchNode->FindChildByID(id2, foundNode))
		{
			return Coral::String::New(std::to_string(foundNode->GetScriptingID()));
		}
	}

	return Coral::String::New(std::to_string(0));
}

bool HasNativeInstanceICall(const Coral::String& canvasUUID, const Coral::String& nodeUUID, const Coral::String& nodeUUID2)
{
	if (!ResourceManager::IsResourceLoaded(UUIDFromString(canvasUUID)))
	{
		Logger::Log("Error finding child, canvas is not loaded." + std::string(canvasUUID), ".net/ui", CRITICAL);
		return UUID(0);
	}

	Ref<UIResource> uiResource = ResourceManager::GetResource<UIResource>(UUIDFromString(canvasUUID));
	Ref<Canvas> canvas = uiResource->GetCanvas();
	if (Ref<Node> node = canvas->GetNodeByUUID(UUIDFromString(nodeUUID));
		node != nullptr)
	{
		if (ScriptingEngineNet::Get().HasCustomWidgetInstance(UUIDFromString(canvasUUID), UUIDFromString(nodeUUID)))
		{
			return true;
		}

		return false;
	}

	return false;
}

Coral::ManagedObject GetNativeInstanceNodeICall(const Coral::String& canvasUUID, const Coral::String& nodeUUID)
{
	if (!ResourceManager::IsResourceLoaded(UUIDFromString(canvasUUID)))
	{
		Logger::Log("Error getting native instance of UI node, canvas is not loaded.", ".net/ui", CRITICAL);
		return Coral::ManagedObject();
	}

	Ref<UIResource> uiResource = ResourceManager::GetResource<UIResource>(UUIDFromString(canvasUUID));
	Ref<Canvas> canvas = uiResource->GetCanvas();
	if(ScriptingEngineNet::Get().HasCustomWidgetInstance(UUIDFromString(canvasUUID), UUIDFromString(nodeUUID)))
	{
		return ScriptingEngineNet::Get().GetCustomWidgetInstance(UUIDFromString(canvasUUID), UUIDFromString(nodeUUID));
	}

	Logger::Log("Error getting native instance of UI node, custom widget doesnt have an instance", ".net/ui", CRITICAL);
	return Coral::ManagedObject();
}

Coral::Bool32 GetVisibilityICall(const Coral::String& canvasUUID, const Coral::String& nodeUUID)
{
	if (!ResourceManager::IsResourceLoaded(UUIDFromString(canvasUUID)))
	{
		Logger::Log("Error getting native instance of UI node, canvas is not loaded.", ".net/ui", CRITICAL);
	}

	Ref<UIResource> uiResource = ResourceManager::GetResource<UIResource>(UUIDFromString(canvasUUID));
	Ref<Canvas> canvas = uiResource->GetCanvas();
	Ref<Node> node = std::static_pointer_cast<NuakeUI::Node>(canvas->GetNodeByUUID(UUIDFromString(nodeUUID)));
	return node->ComputedStyle.Visibility == VisibilityType::Show;
}

void SetWidthPercentageICall(const Coral::String& canvasUUID, const Coral::String& nodeUUID, float percentage)
{
	if (!ResourceManager::IsResourceLoaded(UUIDFromString(canvasUUID)))
	{
		Logger::Log("Error getting native instance of UI node, canvas is not loaded.", ".net/ui", CRITICAL);
	}

	Ref<UIResource> uiResource = ResourceManager::GetResource<UIResource>(UUIDFromString(canvasUUID));
	Ref<Canvas> canvas = uiResource->GetCanvas();
	Ref<Node> node = std::static_pointer_cast<NuakeUI::Node>(canvas->GetNodeByUUID(UUIDFromString(nodeUUID)));
	node->ComputedStyle.Width.type = LengthType::Percentage;
	node->ComputedStyle.Width.value = percentage;
}

void SetHeightPercentageICall(const Coral::String& canvasUUID, const Coral::String& nodeUUID, float percentage)
{
	if (!ResourceManager::IsResourceLoaded(UUIDFromString(canvasUUID)))
	{
		Logger::Log("Error getting native instance of UI node, canvas is not loaded.", ".net/ui", CRITICAL);
	}

	Ref<UIResource> uiResource = ResourceManager::GetResource<UIResource>(UUIDFromString(canvasUUID));
	Ref<Canvas> canvas = uiResource->GetCanvas();
	Ref<Node> node = std::static_pointer_cast<NuakeUI::Node>(canvas->GetNodeByUUID(UUIDFromString(nodeUUID)));
	node->ComputedStyle.Height.type = LengthType::Percentage;
	node->ComputedStyle.Height.value = percentage;
}

void SetVisibilityICall(const Coral::String& canvasUUID, const Coral::String& nodeUUID, Coral::Bool32 visible)
{
	if (!ResourceManager::IsResourceLoaded(UUIDFromString(canvasUUID)))
	{
		Logger::Log("Error getting native instance of UI node, canvas is not loaded.", ".net/ui", CRITICAL);
	}

	Ref<UIResource> uiResource = ResourceManager::GetResource<UIResource>(UUIDFromString(canvasUUID));
	Ref<Canvas> canvas = uiResource->GetCanvas();
	Ref<Node> node = std::static_pointer_cast<NuakeUI::Node>(canvas->GetNodeByUUID(UUIDFromString(nodeUUID)));
	node->ComputedStyle.Visibility = visible ? VisibilityType::Show : VisibilityType::Hidden;
}

Coral::String GetTextNodeTextICall(const Coral::String& canvasUUID, const Coral::String& nodeUUID)
{
	if(!ResourceManager::IsResourceLoaded(UUIDFromString(canvasUUID)))
	{
		Logger::Log("Error getting native instance of UI node, canvas is not loaded.", ".net/ui", CRITICAL);
		return Coral::String::New("");
	}

	Ref<UIResource> uiResource = ResourceManager::GetResource<UIResource>(UUIDFromString(canvasUUID));
	Ref<Canvas> canvas = uiResource->GetCanvas();
	Ref<Text> textNode = std::static_pointer_cast<NuakeUI::Text>(canvas->GetNodeByUUID(UUIDFromString(nodeUUID)));
	return Coral::String::New(textNode->GetText());
}

void SetTextNodeTextICall(const Coral::String& canvasUUID, const Coral::String& nodeUUID, Coral::String newText)
{
	if (!ResourceManager::IsResourceLoaded(UUIDFromString(canvasUUID)))
	{
		Logger::Log("Error getting native instance of UI node, canvas is not loaded.", ".net/ui", CRITICAL);
		return;
	}

	Ref<UIResource> uiResource = ResourceManager::GetResource<UIResource>(UUIDFromString(canvasUUID));
	Ref<Canvas> canvas = uiResource->GetCanvas();
	Ref<Text> textNode = std::static_pointer_cast<NuakeUI::Text>(canvas->GetNodeByUUID(UUIDFromString(nodeUUID)));
	textNode->SetText(newText);
}

void UINetAPI::RegisterMethods()
{
	RegisterMethod("Node.FindChildByIDICall", &FindChildByIDIcall);
	RegisterMethod("Node.HasNativeInstanceICall", &HasNativeInstanceICall);
	RegisterMethod("Node.GetNativeInstanceNodeICall", &GetNativeInstanceNodeICall);

	// Styling
	RegisterMethod("Node.GetVisibilityICall", &GetVisibilityICall);
	RegisterMethod("Node.SetVisibilityICall", &SetVisibilityICall);

	//RegisterMethod("Node.GetWidthPercentageICall", &GetWidthPercentageICall);
	RegisterMethod("Node.SetWidthPercentageICall", &SetWidthPercentageICall);
	//	RegisterMethod("Node.GetHeightPercentageICall", &GetHeightPercentageICall);
	RegisterMethod("Node.SetHeightPercentageICall", &SetHeightPercentageICall);

	RegisterMethod("TextNode.GetTextNodeTextICall", &GetTextNodeTextICall);
	RegisterMethod("TextNode.SetTextNodeTextICall", &SetTextNodeTextICall);
}
#include "UserInterface.h"

#include <src/Vendors/pugixml/pugixml.hpp>
#include "Styling/Stylesheet.h"
#include "yoga/YGConfig.h"
#include "InterfaceParser.h"
#include <src/UI/Font/FontLoader.h>
#include "../Core/Input.h"

namespace UI
{
	UserInterface::UserInterface(const std::string& name)
	{
		m_Name = name;

		font = FontLoader::LoadFont("resources/Fonts/RobotoMono-Regular.ttf");

		m_Stylesheet = StyleSheet::New("/Interface\\Testing.css");

		Root = InterfaceParser::Parse("resources/Interface/Testing.interface");
 		
		if (!Root)
		{
			Logger::Log("Failed to generate interface structure");
		}

		yoga_config = YGConfigNew();
		yoga_config->useWebDefaults = true;
		CreateYogaLayout();
	}

	UserInterface::~UserInterface()
	{
		YGConfigFree(yoga_config);
		if (yoga_root)
			YGNodeFreeRecursive(yoga_root);
	}

	void UserInterface::Reload()
	{
		m_Stylesheet = StyleSheet::New("/Interface\\Testing.css");
		Root = InterfaceParser::Parse("resources/Interface/Testing.interface");
		if (!Root)
		{
			Logger::Log("Failed to generate interface structure");
		}

		yoga_config = YGConfigNew();
		yoga_config->useWebDefaults = true;
		CreateYogaLayout();
	}

	Ref<UserInterface> UserInterface::New(const std::string& name)
	{
		return CreateRef<UserInterface>(name);
	}

	void UserInterface::Calculate(int available_width, int available_height)
	{
		YGNodeRef root = Root->YogaNode;
		YGNodeCalculateLayout(root, available_width, available_height, YGDirectionLTR);
	}

	void UserInterface::CreateYogaLayout()
	{
		yoga_root = YGNodeNewWithConfig(yoga_config);
		Root->YogaNode = yoga_root;
	
		for (auto& g : Root->GetGroups())
			if (m_Stylesheet->HasStyleGroup(g))
				Root->ApplyStyle(m_Stylesheet->GetStyleGroup(g));
		Root->SetYogaLayout();
		CreateYogaLayoutRecursive(Root, yoga_root);
	}


	void UserInterface::CreateYogaLayoutRecursive(Ref<Node> node, YGNodeRef yoga_node)
	{
		if(node->Childrens.size() > 0)
			YGNodeCalculateLayout(yoga_node, YGNodeLayoutGetWidth(yoga_node), YGNodeLayoutGetHeight(yoga_node), YGDirectionLTR);
		int index = 0;
		for (auto& n : node->Childrens)
		{
			YGNodeRef newYogaNode = YGNodeNew();
			n->YogaNode = newYogaNode;

			for (auto& g : n->GetGroups())
				if (m_Stylesheet->HasStyleGroup(g))
					n->ApplyStyle(m_Stylesheet->GetStyleGroup(g));

			n->SetYogaLayout();
			YGNodeInsertChild(yoga_node, newYogaNode, index);
			CreateYogaLayoutRecursive(n, newYogaNode);
			index++;
		}

		Logger::Log(std::to_string(YGNodeGetChildCount(yoga_node)));
	}

	void UserInterface::Draw(Vector2 size)
	{
		Calculate(size.x, size.y);
		Renderer2D::BeginDraw(size);

		float leftOffset = YGNodeLayoutGetLeft(Root->YogaNode);
		float topOffset = YGNodeLayoutGetTop(Root->YogaNode);
		

		Vector2 charPos = Vector2(100.f, 100.f);
		Char charr = font->GetChar(89);

		

		DrawRecursive(Root, 0);
		//Renderer2D::DrawChar(charr, font, charPos, size);
	}

	void UserInterface::DrawRecursive(Ref<Node> node, float z)
	{
		if (!node)
			return;

		if (node->Type == NodeType::Text)
			std::static_pointer_cast<TextNode>(node)->Draw(z);
		else
			node->Draw(z);
		
		if (node->Childrens.size() <= 0)
			return;

		for (auto& c : node->Childrens)
		{
			DrawRecursive(c, z + 1);
		}
	}

	void UserInterface::Update(Timestep ts)
	{
		// Check Input
		if (Input::IsMouseButtonPressed(0))
		{
			ConsumeMouseClick(Input::GetMousePosition());
		}
	}


	void UserInterface::RecursiveMouseClick(Ref<Node> node, Vector2 pos)
	{
		for (auto& c : node->Childrens)
		{
			RecursiveMouseClick(c, pos);
			if (c->IsPositionInside(pos) && c->OnClickSignature != "")
				this->Root->Script->CallMethod(c->OnClickSignature);

		}
	}

	void UserInterface::ConsumeMouseClick(Vector2 pos)
	{

		RecursiveMouseClick(Root, pos);
	}
}

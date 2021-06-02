#include "UserInterface.h"

#include <src/Vendors/pugixml/pugixml.hpp>
#include "Stylesheet.h"
#include "yoga/YGConfig.h"
#include "InterfaceParser.h"
namespace UI
{
	UserInterface::UserInterface(const std::string& name)
	{
		m_Name = name;

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
						  
		DrawRecursive(Root, 0, Vector2(leftOffset, topOffset));
	}

	void UserInterface::DrawRecursive(Ref<Node> node, float z, Vector2 offset)
	{
		if (!node)
			return;

		node->Draw(z, offset);
		
		if (node->Childrens.size() <= 0)
			return;

		offset.x += YGNodeLayoutGetLeft(Root->YogaNode);

		if(!YGNodeLayoutGetHadOverflow(Root->YogaNode))
			offset.y += YGNodeLayoutGetTop(Root->YogaNode);

		for (auto& c : node->Childrens)
		{
			DrawRecursive(c, z + 1, offset);
		}
	}

	void UserInterface::Update(Timestep ts)
	{

	}
}

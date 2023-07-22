#include "UserInterface.h"

#include "src/Vendors/pugixml/pugixml.hpp"
#include "Styling/Stylesheet.h"
#include "yoga/YGConfig.h"
#include "InterfaceParser.h"
#include <src/UI/Font/FontLoader.h>
#include "src/Core/Input.h"

namespace Nuake {
	namespace UI {
		UserInterface::UserInterface(const std::string& name, const std::string& path)
		{
			m_Name = name;
			m_Path = path;

			font = FontLoader::LoadFont("resources/Fonts/OpenSans-Regular.ttf");
			Root = InterfaceParser::Parse(FileSystem::Root + path);

			if (!Root)
			{
				Logger::Log("Failed to generate interface structure", "ui", CRITICAL);
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
			Root = InterfaceParser::Parse(FileSystem::Root + this->m_Path);
			if (!Root)
			{
				Logger::Log("Failed to generate interface structure", "ui", CRITICAL);
			}

			yoga_config = YGConfigNew();
			yoga_config->useWebDefaults = true;
			CreateYogaLayout();
		}

		Ref<UserInterface> UserInterface::New(const std::string& name, const std::string& path)
		{
			return CreateRef<UserInterface>(name, path);
		}

		void UserInterface::Calculate(int available_width, int available_height)
		{
			YGNodeRef root = Root->YogaNode;
			YGNodeCalculateLayout(root, static_cast<float>(available_width), static_cast<float>(available_height), YGDirectionLTR);
		}

		void UserInterface::CreateYogaLayout()
		{
			yoga_root = YGNodeNewWithConfig(yoga_config);
			Root->YogaNode = yoga_root;

			for (auto& g : Root->GetGroups())
			{
				if (!Root->StyleSheet->HasStyleGroup(g))
					continue;

				Ref<StyleGroup> stylegroup = Root->StyleSheet->GetStyleGroup(g);
				if(stylegroup->Selector == StyleGroupSelector::Normal)
					Root->ApplyStyle(stylegroup);
			}
			Root->SetYogaLayout();
			CreateYogaLayoutRecursive(Root, yoga_root);
		}

		void UserInterface::CreateYogaLayoutRecursive(Ref<Node> node, YGNodeRef yoga_node)
		{
			if (node->Childrens.size() > 0)
				YGNodeCalculateLayout(yoga_node, YGNodeLayoutGetWidth(yoga_node), YGNodeLayoutGetHeight(yoga_node), YGDirectionLTR);
			
			int index = 0;
			for (auto& n : node->Childrens)
			{
				YGNodeRef newYogaNode = YGNodeNew();
				n->YogaNode = newYogaNode;

				for (auto& g : n->GetGroups())
				{
					if (!Root->StyleSheet->HasStyleGroup(g))
						continue;

					Ref<StyleGroup> stylegroup = Root->StyleSheet->GetStyleGroup(g);
					if (stylegroup->Selector == StyleGroupSelector::Normal)
						n->ApplyStyle(stylegroup);
				}

				n->HoverStyle = n->NormalStyle;

				n->SetYogaLayout();
				YGNodeInsertChild(yoga_node, newYogaNode, index);
				CreateYogaLayoutRecursive(n, newYogaNode);
				index++;
			}
		}

		void UserInterface::Draw(Vector2 size)
		{
			Calculate((int)size.x, (int)size.y);
			Renderer2D::BeginDraw(size);
			DrawRecursive(Root, 0);
			Renderer2D::EndDraw();
			Size = size;
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
				DrawRecursive(c, z + 1);
		}

		void UserInterface::Update(Timestep ts)
		{
			// Hover
			RecursiveHover(Root, Input::GetMousePosition());

			// Check Input
			ConsumeMouseClick(Input::GetMousePosition());
		}

		void UserInterface::RecursiveHover(Ref<Node> node, Vector2 pos)
		{
			for (auto& c : node->Childrens)
			{
				RecursiveHover(c, pos);

				bool isHover = c->IsPositionInside(pos);
				c->IsHover = isHover;
				c->IsClicked = false;
				if (isHover)
				{
					for (auto& g : c->GetGroups())
					{
						if (!Root->StyleSheet->HasStyleGroup(g + ":hover"))
							continue;

						Ref<StyleGroup> stylegroup = Root->StyleSheet->GetStyleGroup(g + ":hover");
						c->ApplyStyle(stylegroup, StyleGroupSelector::Hover);
					}
				}
				c->SetYogaLayout();
			}
		}

		void UserInterface::RecursiveMouseClick(Ref<Node> node, Vector2 pos)
		{
			if (node->IsHover)
			{
				node->IsClicked = true;
			}

			for (auto& c : node->Childrens)
			{
				RecursiveMouseClick(c, pos);
			}
		}

		void UserInterface::ConsumeMouseClick(Vector2 pos)
		{
			if(Input::IsMouseButtonPressed(0))
				RecursiveMouseClick(Root, pos);
		}


		Ref<Node> UserInterface::GetNodeByID(const std::string& id)
		{
			for (auto& n : Root->Childrens)
			{
				Ref<Node> result = GetNodeByIDRecurse(n, id);
				if (result)
					return result;
			}

			return nullptr;
		}

		Ref<Node> UserInterface::GetNodeByIDRecurse(Ref<Node> node, const std::string& id)
		{
			for (auto& n : node->Childrens)
			{
				if (n->ID == id)
					return n;

				Ref<Node> result = GetNodeByIDRecurse(n, id);
				if (result != nullptr)
					return result;
			}
			return nullptr;
		}
	}
}


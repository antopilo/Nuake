#include "InterfaceParser.h"

Ref<Canvas> InterfaceParser::Root = CreateRef<Canvas>();

void InterfaceParser::Iterate(const pugi::xml_node& xml_node, Ref<Node> node)
{
	for (auto& e : xml_node.children())
	{
		std::string type = e.name();
		if (type == "Canvas")
		{
			Ref<Canvas> canvas = CreateCanvas(e);
			node->Childrens.push_back(canvas);
			Iterate(e, canvas);
		}
		else if (type == "Rect")
		{
			Ref<Node> newNode = CreateNode(e);
			node->Childrens.push_back(newNode);
			Iterate(e, newNode);
		}
		else if (type == "p")
		{
			Ref<TextNode> textnode = CreateTextNode(e);
			node->Childrens.push_back(textnode);
			Iterate(e, textnode);
		}
	}
}
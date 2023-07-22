#include "InterfaceParser.h"
#include "Styling/Stylesheet.h"
#include "src/Core/String.h"
namespace Nuake
{
	Ref<Canvas> InterfaceParser::Root = CreateRef<Canvas>();

	Ref<Canvas> InterfaceParser::Parse(const std::string path)
	{
		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_file(path.c_str());
		std::string name = doc.first_child().name();
		if (name != "Canvas")
		{
			Logger::Log("InterfaceParser error: First child should be a canvas - " + path, "ui", CRITICAL);
			return nullptr;
		}

		Root = CreateCanvas(doc.first_child());
		Root->Depth = 0;
		Iterate(doc.first_child(), Root, 0);
		return Root;
	}

	void InterfaceParser::Iterate(const pugi::xml_node& xml_node, Ref<Node> node, int depth)
	{
		int currentDepth = depth + 1;
		for (auto& e : xml_node.children())
		{
			std::string type = e.name();
			Ref<Node> newNode;

			if (type == "Canvas")
				newNode = CreateCanvas(e);
			else if (type == "Rect")
				newNode = CreateNode(e);
			else if (type == "p")
				newNode = CreateTextNode(e);
			else
				continue;
			newNode->Depth = currentDepth;
			node->Childrens.push_back(newNode);
			Iterate(e, newNode, currentDepth);
		}
	}

	Ref<TextNode> InterfaceParser::CreateTextNode(const pugi::xml_node& xml_node)
	{
		Ref<TextNode> node = CreateRef<TextNode>();
		node->content = xml_node.text().as_string();
		node->Type = NodeType::Text;
		TextStyle style {
			FontManager::GetFont("resources/Fonts/RobotoMono-Regular.ttf"),
			0.5,
			Color(255, 255, 255, 255)
		};
		
		for (auto& a : xml_node.attributes())
		{
			std::string name = a.name();
			if (name == "groups")
				for (auto& g : String::Split(a.value(), ' '))
					node->AddGroup(g);

			if (name == "id")
				node->ID = a.value();
			if (name == "height")
				node->NormalStyle.Height = GetUnit(a.value());
			if (name == "width")
				node->NormalStyle.Width = GetUnit(a.value());
			if (name == "margin")
				node->NormalStyle.Margin = GetVec4Unit(a.value());
			if (name == "padding")
				node->NormalStyle.Padding = GetVec4Unit(a.value());
			if (name == "border")
				node->NormalStyle.Border = GetVec4Unit(a.value());
			if (name == "position")
				node->NormalStyle.Position = GetVec4Unit(a.value());
			if (name == "color")
				node->NormalStyle.BackgroundColor = GetColor(a.value());
			if (name == "font-size")
				style.fontSize = std::stof(a.value());
		}

		Vector2 minSize = Renderer2D::CalculateStringSize(node->content, style);
		node->NormalStyle.Width = {
			minSize.x,
			Layout::Unit::PIXEL
		};
		node->NormalStyle.Height = {
			minSize.y,
			Layout::Unit::PIXEL
		};


		node->SetTextStyle(style);
		return node;
	}

	Ref<Node> InterfaceParser::CreateNode(const pugi::xml_node& xml_node)
	{
		Ref<Node> node = CreateRef<Node>();

		node->Root = Root;
		for (auto& a : xml_node.attributes())
		{
			std::string name = a.name();
			if (name == "groups")
				for (auto& g : String::Split(a.value(), ' '))
					node->AddGroup(g);

			if (name == "id")
				node->ID = a.value();
			if (name == "height")
				node->NormalStyle.Height = GetUnit(a.value());
			if (name == "width")
				node->NormalStyle.Width = GetUnit(a.value());
			if (name == "margin")
				node->NormalStyle.Margin = GetVec4Unit(a.value());
			if (name == "padding")
				node->NormalStyle.Padding = GetVec4Unit(a.value());
			if (name == "border")
				node->NormalStyle.Border = GetVec4Unit(a.value());
			if (name == "position")
				node->NormalStyle.Position = GetVec4Unit(a.value());
			if (name == "color")
				node->NormalStyle.BackgroundColor = GetColor(a.value());
			if (name == "onclick")
			{
				//std::string signature = a.value();
				//Root->Script->RegisterMethod(a.value());
				//node->OnClickSignature = a.value();
			}
		}

		return node;
	}

	Layout::LayoutUnit InterfaceParser::GetUnit(const std::string& value)
	{
		std::regex only_number("[0-9]+");
		std::regex not_number("[^0-9]+");

		// is percentage
		std::smatch match_value;
		if (std::regex_search(value.begin(), value.end(), match_value, only_number))
		{
			std::smatch match_type;
			if (std::regex_search(value.begin(), value.end(), match_type, not_number))
			{
				float value = std::stof(match_value[0]);

				if (match_type[0] == "%")
				{
					return Layout::LayoutUnit{
						value,
						Layout::Unit::PERCENT
					};
				}
				else if (match_type[0] == "px")
				{
					return Layout::LayoutUnit{
						value,
						Layout::Unit::PIXEL
					};
				}
			}
		}

		return Layout::LayoutUnit{ 1.0f, Layout::Unit::AUTO };
	}

	Layout::LayoutVec4 InterfaceParser::GetVec4Unit(const std::string& value)
	{
		Layout::LayoutVec4 result;

		std::regex regex("[0-9]+[^ ]+");
		std::smatch match_value;

		std::vector<std::string> splits = String::Split(value, ' ');
		for (int i = 0; i < splits.size(); i++)
		{
			if (!std::regex_search(value.begin(), value.end(), match_value, regex))
				continue;

			if (i == 0)
				result.Left = GetUnit(splits[i]);
			else if (i == 1)
				result.Right = GetUnit(splits[i]);
			else if (i == 2)
				result.Top = GetUnit(splits[i]);
			else
				result.Bottom = GetUnit(splits[i]);
		}
		
		return result;
	}

	Color InterfaceParser::GetColor(const std::string& value)
	{
		Color result;

		std::regex regex("[0-9]+[^ ]+");
		std::smatch match_value;

		std::vector<std::string> splits = String::Split(value, ' ');
		for (int i = 0; i < splits.size(); i++)
		{
			if (!std::regex_search(value.begin(), value.end(), match_value, regex))
				continue;

			if (i == 0) result.r = std::stof(splits[i]);
			else if (i == 1) result.g = std::stof(splits[i]);
			else if (i == 2) result.b = std::stof(splits[i]);
			else result.a = std::stof(splits[i]);
		}

		return result;
	}

	Ref<Canvas> InterfaceParser::CreateCanvas(const pugi::xml_node& xml_node)
	{
		Ref<Canvas> node = CreateRef<Canvas>();
		node->Root = Root;
		for (auto& a : xml_node.attributes())
		{
			std::string name = a.name();
			if (name == "groups")
				for (auto& g : String::Split(a.value(), ' '))
					node->AddGroup(g);
			if (name == "id")
				node->ID = a.value();
			else if (name == "height")
				node->NormalStyle.Height = GetUnit(a.value());
			else if (name == "width")
				node->NormalStyle.Width = GetUnit(a.value());
			else if (name == "margin")
				node->NormalStyle.Margin = GetVec4Unit(a.value());
			else if (name == "padding")
				node->NormalStyle.Padding = GetVec4Unit(a.value());
			else if (name == "border")
				node->NormalStyle.Border = GetVec4Unit(a.value());
			else if (name == "position")
				node->NormalStyle.Position = GetVec4Unit(a.value());
			else if (name == "color")
				node->NormalStyle.BackgroundColor = GetColor(a.value());
			else if (name == "script")
			{
				auto scriptModule = String::Split(a.value(), ' ');
				node->ScriptsToLoad.push_back({ scriptModule[0], scriptModule[1] });
				//node->Script = ScriptingEngine::RegisterScript(FileSystem::Root + path, module);
			}
			else if (name == "stylesheet")
			{
				std::string path = a.value();
				node->StyleSheet = UI::StyleSheet::New(FileSystem::Root + path);
			}
		}

		return node;
	}
}

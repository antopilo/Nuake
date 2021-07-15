#include "InterfaceParser.h"
#include "Styling/Stylesheet.h"

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
			Logger::Log("InterfaceParser error: First child should be a canvas - " + path, CRITICAL);
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
		TextStyle style{
			FontManager::GetFont("resources/Fonts/RobotoMono-Regular.ttf"),
			0.5,
			Color(255, 255, 255, 255)
		};
		
		for (auto& a : xml_node.attributes())
		{
			std::string name = a.name();
			if (name == "groups")
			{
				for (auto& g : split(a.value(), ' '))
				{
					node->AddGroup(g);
				}
			}
			if (name == "height")
				node->Height = GetUnit(a.value());
			if (name == "width")
				node->Width = GetUnit(a.value());
			if (name == "margin")
				node->Margin = GetVec4Unit(a.value());
			if (name == "padding")
				node->Padding = GetVec4Unit(a.value());
			if (name == "border")
				node->Border = GetVec4Unit(a.value());
			if (name == "position")
				node->Position = GetVec4Unit(a.value());
			if (name == "color")
				node->BackgroundColor = GetColor(a.value());
			if (name == "font-size")
				style.fontSize = std::stof(a.value());
		}

		Vector2 minSize = Renderer2D::CalculateStringSize(node->content, style);
		node->Width = {
			minSize.x,
			Layout::Unit::PIXEL
		};
		node->Height = {
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
			{
				for (auto& g : split(a.value(), ' '))
				{
					node->AddGroup(g);
				}
			}
			if (name == "height")
				node->Height = GetUnit(a.value());
			if (name == "width")
				node->Width = GetUnit(a.value());
			if (name == "margin")
				node->Margin = GetVec4Unit(a.value());
			if (name == "padding")
				node->Padding = GetVec4Unit(a.value());
			if (name == "border")
				node->Border = GetVec4Unit(a.value());
			if (name == "position")
				node->Position = GetVec4Unit(a.value());
			if (name == "color")
				node->BackgroundColor = GetColor(a.value());
			if (name == "onclick")
			{
				std::string signature = a.value();
				Root->Script->RegisterMethod(a.value());
				node->OnClickSignature = a.value();
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
	}

	std::vector<std::string> InterfaceParser::split(std::string const& str, const char delim)
	{
		std::vector<std::string> result;
		size_t start;
		size_t end = 0;

		while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
		{
			end = str.find(delim, start);
			result.push_back(str.substr(start, end - start));
		}

		return result;
	}

	Layout::LayoutVec4 InterfaceParser::GetVec4Unit(const std::string& value)
	{
		std::regex regex("[0-9]+[^ ]+");
		std::smatch match_value;
		Layout::LayoutVec4 result;

		std::vector<std::string> splits = split(value, ' ');
		int idx = 0;
		for (auto& s : splits)
		{
			if (std::regex_search(value.begin(), value.end(), match_value, regex))
			{
				if (idx == 0)
				{
					result.Left = GetUnit(s);
				}
				else if (idx == 1)
				{
					result.Right = GetUnit(s);
				}
				else if (idx == 2)
				{
					result.Top = GetUnit(s);
				}
				else
				{
					result.Bottom = GetUnit(s);
				}
				idx++;
			}
		}
		return result;
	}

	Color InterfaceParser::GetColor(const std::string& value)
	{
		std::regex regex("[0-9]+[^ ]+");
		std::smatch match_value;
		Color result;

		std::vector<std::string> splits = split(value, ' ');
		int idx = 0;
		for (auto& s : splits)
		{
			if (std::regex_search(value.begin(), value.end(), match_value, regex))
			{
				if (idx == 0)
				{
					result.r = std::stof(s);
				}
				else if (idx == 1)
				{
					result.g = std::stof(s);
				}
				else if (idx == 2)
				{
					result.b = std::stof(s);
				}
				else
				{
					result.a = std::stof(s);
				}
				idx++;
			}
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
			{
				for (auto& g : split(a.value(), ' '))
				{
					node->AddGroup(g);
				}
			}
			if (name == "height")
				node->Height = GetUnit(a.value());
			if (name == "width")
				node->Width = GetUnit(a.value());
			if (name == "margin")
				node->Margin = GetVec4Unit(a.value());
			if (name == "padding")
				node->Padding = GetVec4Unit(a.value());
			if (name == "border")
				node->Border = GetVec4Unit(a.value());
			if (name == "position")
				node->Position = GetVec4Unit(a.value());
			if (name == "color")
				node->BackgroundColor = GetColor(a.value());
			if (name == "script")
			{
				auto s = split(a.value(), ' ');
				std::string path = s[0];
				std::string module = s[1];
				node->Script = ScriptingEngine::RegisterScript(path, module);
			}
			if (name == "stylesheet")
			{
				std::string path = a.value();
				node->StyleSheet = UI::StyleSheet::New(path);
			}
		}

		return node;
	}
}

#pragma once
#include <src/Vendors/pugixml/pugixml.hpp>
#include <iostream>
#include <src/UI/Nodes/Canvas.h>
#include <src/UI/Nodes/Rect.h>
#include <regex>
#include<iostream>
#include <src/Core/Logger.h>
#include "Nodes/TextNode.h"
#include "Font/FontManager.h"
class InterfaceParser
{
	static Ref<Canvas> Root;

private:

public:
	static Ref<Canvas> Parse(const std::string path)
	{
		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_file(path.c_str());

		std::string name = doc.first_child().name();
		if (name != "Canvas")
		{
			Logger::Log("InterfaceParser error: First child should be a canvas - " + path);
			return nullptr;
		}
			
		Root = CreateCanvas(doc.first_child());

		Iterate(doc.first_child(), Root);
		return Root;
	}



	static void Iterate(const pugi::xml_node& xml_node, Ref<Node> node);

	static Ref<TextNode> CreateTextNode(const pugi::xml_node& xml_node)
	{
		Ref<TextNode> node = CreateRef<TextNode>();
		node->content = xml_node.text().as_string();

		TextStyle style{
			FontManager::GetFont("resources/Font/OpenSans-Regular.ttf"),
			2.0,
			Color(255, 255, 255, 255)
		};

		node->SetTextStyle(style);
		return node;
	}

	static Ref<Node> CreateNode(const pugi::xml_node& xml_node)
	{
		Ref<Node> node = CreateRef<Node>();
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
		}

		return node;
	}

	static Layout::LayoutUnit GetUnit(const std::string& value)
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

	static std::vector<std::string> split(std::string const& str, const char delim)
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

	static Layout::LayoutVec4 GetVec4Unit(const std::string& value)
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

	static Color GetColor(const std::string& value)
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


	static Ref<Canvas> CreateCanvas(const pugi::xml_node& xml_node)
	{
		Ref<Canvas> node = CreateRef<Canvas>();
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

		}

		return node;

	}
};
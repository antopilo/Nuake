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
#include "../Scripting/ScriptingEngine.h"
class InterfaceParser
{
private:
	static Ref<Canvas> Root;
public:
	static Ref<Canvas> Parse(const std::string path);

	static void Iterate(const pugi::xml_node& xml_node, Ref<Node> node, int depth);

	static Ref<Node> CreateNode(const pugi::xml_node& xml_node);
	static Ref<Canvas> CreateCanvas(const pugi::xml_node& xml_node);
	static Ref<TextNode> CreateTextNode(const pugi::xml_node& xml_node);

	static std::vector<std::string> split(std::string const& str, const char delim);

	static Layout::LayoutUnit GetUnit(const std::string& value);
	static Layout::LayoutVec4 GetVec4Unit(const std::string& value);
	static Color GetColor(const std::string& value);
};
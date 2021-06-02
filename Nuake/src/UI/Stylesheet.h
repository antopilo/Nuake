#pragma once

#include "../Core/FileSystem.h"
#include "../Core/Logger.h"
#include "../Core/Core.h"
#include "Style.h"

#include "katana-parser/katana.h"
#include <string>
#include <map>
#include <src/UI/StyleSheetParser.h>
#include <regex>
#include <regex>
#include <src/UI/InterfaceParser.h>
#include <src/UI/InterfaceParser.h>

namespace UI
{
	class StyleSheet
	{
	private:
		KatanaOutput* Data;
		
		std::map<std::string, Ref<StyleGroup>> Styles;
	public:
		std::string Path;
		static Ref<StyleSheet> New(const std::string& path);

		void AddStyleGroup(std::string selector, Ref<StyleGroup> group)
		{
			Styles[selector] = group;
		}

		bool HasStyleGroup(const std::string& group)
		{
			return Styles.find(group) != Styles.end();
		}

		Ref<StyleGroup> GetStyleGroup(const std::string group)
		{
			if (!HasStyleGroup(group))
				return nullptr;

			return Styles[group];
		}

		StyleSheet(const std::string& path)
		{
			this->Path = path;
			ParseSheet();
		}

		~StyleSheet()
		{
			katana_destroy_output(Data);
		}

		bool ParseRule(KatanaRule* rule)
		{
			if (rule->type == KatanaRuleStyle)
			{
				Ref<StyleGroup> styleGroup = CreateRef<StyleGroup>();

				// Selectors
				KatanaStyleRule* stylerule = (KatanaStyleRule*)rule;
				for (int j = 0; j < stylerule->selectors->length; j++)
				{
					KatanaSelector* selector = (KatanaSelector*)stylerule->selectors->data[j];
					std::string name = selector->data->value;

					AddStyleGroup(name, styleGroup);
				}

				// Declarations
				for (int k = 0; k < stylerule->declarations->length; k++)
				{
					KatanaDeclaration* declaration = (KatanaDeclaration*)(stylerule->declarations->data[k]);
					std::string value = declaration->raw;
					std::string name = declaration->property;
					PropType type;

					if (name == "height") type = PropType::HEIGHT;
					if (name == "width")type = PropType::WIDTH;

					if (name == "margin")
					{
						std::regex regex("[0-9]+[^ ]+");
						std::smatch match_value;
						Layout::LayoutVec4 result;

						std::vector<std::string> splits = InterfaceParser::split(value, ' ');
						int idx = 0;
						for (auto& s : splits)
						{
							if (idx == 0)
							{
								type = PropType::MARGIN_LEFT;
								styleGroup->SetProp(type, StyleSheetParser::ParsePropType(s, type));
							}
							else if (idx == 1)
							{
								type = PropType::MARGIN_RIGHT;
								styleGroup->SetProp(type, StyleSheetParser::ParsePropType(s, type));
							}
							else if (idx == 2)
							{
								type = PropType::MARGIN_TOP;
								styleGroup->SetProp(type, StyleSheetParser::ParsePropType(s, type));
							}
							else if (idx == 3)
							{
								type = PropType::MARGIN_BOTTOM;
								styleGroup->SetProp(type, StyleSheetParser::ParsePropType(s, type));
							}
							idx++;
						}
						return true;
					}
					styleGroup->SetProp(type, StyleSheetParser::ParsePropType(value, type));
					Logger::Log(declaration->property);
				}
				Logger::Log(rule->name);
			}
			return true;
		}
		bool ParseSheet()
		{
			std::string content = FileSystem::ReadFile(this->Path);
			Data = katana_parse(content.c_str(), content.length(), KatanaParserModeStylesheet);
			for (int i = 0; i < Data->stylesheet->rules.length; i++)
			{
				KatanaRule* rule = (KatanaRule*)Data->stylesheet->rules.data[i];
				ParseRule(rule);
			}
			
			if (Data->errors.length > 0)
			{
				KatanaArray errors = Data->errors;
				return false;
			}
			katana_dump_output(Data);
			return true;
		}
	};
}
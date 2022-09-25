#pragma once

#include "katana-parser/katana.h"
#include "../Core/FileSystem.h"
#include "../Core/Logger.h"
#include "../Core/Core.h"
#include "../Nodes/Node.h"
#include <src/UI/Styling/StyleSheetParser.h>
#include <regex>
#include "src/Core/String.h"
#include <map>

namespace Nuake
{
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
					styleGroup->Selector = StyleGroupSelector::Normal;

					// Selectors
					KatanaStyleRule* stylerule = (KatanaStyleRule*)rule;
					for (uint32_t j = 0; j < stylerule->selectors->length; j++)
					{
						KatanaSelector* selector = (KatanaSelector*)stylerule->selectors->data[j];
						std::string name = selector->data->value;

						KatanaPseudoType pseudo = KatanaPseudoUnknown;
						if(selector->tagHistory)
							pseudo = selector->tagHistory->pseudo;

						// Special selectors
						if (pseudo == KatanaPseudoHover)
						{
							name += ":hover";
							styleGroup->Selector = StyleGroupSelector::Hover;
						}
						
						AddStyleGroup(name, styleGroup);
					}

					// Declarations
					for (uint32_t k = 0; k < stylerule->declarations->length; k++)
					{
						KatanaDeclaration* declaration = (KatanaDeclaration*)(stylerule->declarations->data[k]);
						std::string value = declaration->raw;
						std::string name = declaration->property;
						PropType type = PropType::NONE;

						if (name == "height") type = PropType::HEIGHT;
						if (name == "width") type = PropType::WIDTH;
						if (name == "left") type = PropType::LEFT;
						if (name == "right") type = PropType::RIGHT;
						if (name == "top") type = PropType::TOP;
						if (name == "bottom") type = PropType::BOTTOM;
						if (name == "margin")
						{
							std::regex regex("[0-9]+[^ ]+");
							std::smatch match_value;
							Layout::LayoutVec4 result;

							std::vector<std::string> splits = String::Split(value, ' ');
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
							continue;
						}
						if (name == "flex-direction")
						{
							type = PropType::FLEX_DIRECTION;
							Value propValue{};

							if (value == "row")
								propValue.Enum = (int)Layout::FlexDirection::ROW;
							if (value == "row-reversed")
								propValue.Enum = (int)Layout::FlexDirection::ROW_REVERSED;
							if (value == "column")
								propValue.Enum = (int)Layout::FlexDirection::COLUMN;
							if (value == "column-reversed")
								propValue.Enum = (int)Layout::FlexDirection::COLUMN_REVERSED;

							styleGroup->SetProp(type, PropValue{ PropValueType::ENUM, propValue });
							continue;
						}
						if (name == "justify-content")
						{
							type = PropType::JUSTIFY_CONTENT;
							Value propValue{};
							if (value == "flex-start")
								propValue.Enum = (int)Layout::JustifyContent::FLEX_START;
							if (value == "flex-end")
								propValue.Enum = (int)Layout::JustifyContent::FLEX_END;
							if (value == "center")
								propValue.Enum = (int)Layout::JustifyContent::CENTER;
							if (value == "space-between")
								propValue.Enum = (int)Layout::JustifyContent::SPACE_BETWEEN;
							if (value == "space-around")
								propValue.Enum = (int)Layout::JustifyContent::SPACE_AROUND;
							if (value == "space-evenly")
								propValue.Enum = (int)Layout::JustifyContent::SPACE_EVENLY;

							styleGroup->SetProp(type, PropValue{ PropValueType::ENUM, propValue });
							continue;
						}
						if (name == "flex-wrap")
						{
							type = PropType::FLEX_WRAP;
							Value propValue{};
							if (value == "no-wrap")
								propValue.Enum = (int)Layout::FlexWrap::NO_WRAP;
							if (value == "wrap")
								propValue.Enum = (int)Layout::FlexWrap::WRAP;
							if (value == "wrap-reversed")
								propValue.Enum = (int)Layout::FlexWrap::WRAP_REVERSED;

							styleGroup->SetProp(type, PropValue{ PropValueType::ENUM, propValue });
							continue;
						}
						if (name == "align-self" || name == "align-items")
						{
							if (name == "align-self")
								type = PropType::SELF_ALIGN;
							else
								type = PropType::ALIGN_ITEMS;

							Value propValue{};
							if (value == "stretch")
								propValue.Enum = (int)Layout::AlignItems::STRETCH;
							if (value == "flex-start")
								propValue.Enum = (int)Layout::AlignItems::FLEX_START;
							if (value == "flex-end")
								propValue.Enum = (int)Layout::AlignItems::FLEX_END;
							if (value == "center")
								propValue.Enum = (int)Layout::AlignItems::CENTER;
							if (value == "baseline")
								propValue.Enum = (int)Layout::AlignItems::BASELINE;

							styleGroup->SetProp(type, PropValue{ PropValueType::ENUM, propValue });
							continue;
						}
						if (name == "align-content")
						{
							type = PropType::ALIGN_CONTENT;

							Value propValue{};
							if (value == "stretch")
								propValue.Enum = (int)Layout::AlignContent::STRETCH;
							if (value == "flex-start")
								propValue.Enum = (int)Layout::AlignContent::FLEX_START;
							if (value == "flex-end")
								propValue.Enum = (int)Layout::AlignContent::FLEX_END;
							if (value == "center")
								propValue.Enum = (int)Layout::AlignContent::CENTER;
							if (value == "space-between")
								propValue.Enum = (int)Layout::AlignContent::SPACE_BETWEEN;
							if (value == "space-around")
								propValue.Enum = (int)Layout::AlignContent::SPACE_AROUND;
							styleGroup->SetProp(type, PropValue{ PropValueType::ENUM, propValue });
							continue;
						}
						if (name == "position")
						{
							type = PropType::POSITION;
							Value propValue{};
							if (value == "relative")
								propValue.Enum = (int)Layout::PositionType::RELATIVE;
							if (value == "absolute")
								propValue.Enum = (int)Layout::PositionType::ABSOLUTE;

							styleGroup->SetProp(type, PropValue{ PropValueType::ENUM, propValue });
							continue;
						}
						if (name == "font-size")
						{
							type = PropType::FONT_SIZE;
						}
						if (name == "background-color")
						{
							type = PropType::BACKGROUND_COLOR;
						}

						if(type != PropType::NONE)
							styleGroup->SetProp(type, StyleSheetParser::ParsePropType(value, type));
					}
				}
				return true;
			}
			bool ParseSheet()
			{
				std::string content = FileSystem::ReadFile(this->Path, true);
				Data = katana_parse(content.c_str(), content.length(), KatanaParserModeStylesheet);
				for (uint32_t i = 0; i < Data->stylesheet->rules.length; i++)
				{
					KatanaRule* rule = (KatanaRule*)Data->stylesheet->rules.data[i];
					ParseRule(rule);
				}

				if (Data->errors.length > 0)
				{
					KatanaArray errors = Data->errors;
					return false;
				}
				return true;
			}
		};
	}
}

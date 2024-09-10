#include "StyleSheetParser.h"

#include <cassert>
#include <vector>

#include "../FileSystem.h"
#include <iostream>

namespace NuakeUI
{
	std::shared_ptr<StyleSheet> StyleSheetParser::Parse(const std::string& path)
	{
		assert(FileSystem::FileExists(path));

		_parsingPath = path;

		std::string fileContent = FileSystem::ReadFile(path);
		auto data = katana_parse(fileContent.c_str(), fileContent.length(), KatanaParserModeStylesheet);

		auto styleSheet = StyleSheet::New();
		// Print out errors.
		if (data->errors.length > 0)
		{
			KatanaArray errors = data->errors;
			for (uint32_t i = 0; i < errors.length; i++)
			{
				KatanaError* error = (KatanaError*)errors.data[i];
				std::cout << "Failed to parse css file \"" + path + "\"." << std::endl;
				std::cout << "Error is " << error->message << std::endl;
				std::cout << "ERROR at line " + std::to_string(error->first_line) +
					" : " + std::to_string(error->first_column) << std::endl;
			}
			return styleSheet;
		}
		else
		{
			ParseRules(data->stylesheet, styleSheet);
		}
		
		_visitedFiles.clear();

		return styleSheet;
	}

	bool StyleSheetParser::FileAlreadyVisited(const std::string& path)
	{
		return std::find(_visitedFiles.begin(), _visitedFiles.end(), path) != _visitedFiles.end();
	}

	void StyleSheetParser::ParseRules(KatanaStylesheet* katanaStylesheet, StyleSheetPtr stylesheet)
	{
		// Import files first
		auto imports = katanaStylesheet->imports;
		for (uint32_t i = 0; i < imports.length; i++)
		{
			KatanaImportRule* importRule = static_cast<KatanaImportRule*>(imports.data[i]);
			ParseImportRule(importRule, stylesheet);
		}

		// Parse generic rules
		auto rules = katanaStylesheet->rules;
		for (uint32_t i = 0; i < rules.length; i++)
		{
			KatanaRule* rule = (KatanaRule*)rules.data[i];

			auto ruleType = rule->type;
			switch (ruleType)
			{
			case KatanaRuleStyle: // Not sure if needed.
				ParseStyleRule(rule, stylesheet);
				break;
			}
		}
	}

	void StyleSheetParser::ParseImportRule(KatanaImportRule* rule, StyleSheetPtr styleSheet)
	{
		std::string path = rule->href;

		if (FileAlreadyVisited(path))
		{
			std::cout << "Cyclic file import detected! " << "File is: " << path << std::endl;
			return;
		}

		_visitedFiles.push_back(path);

		if (!FileSystem::FileExists(path))
		{
			std::cout << "CSS Import rule error: Cannot find file: " << path << std::endl;
			return;
		}

		std::string fileContent = FileSystem::ReadFile(path);
		auto data = katana_parse(fileContent.c_str(), fileContent.length(), KatanaParserModeStylesheet);

		if (data->errors.length > 0)
		{
			KatanaArray errors = data->errors;
			for (uint32_t i = 0; i < errors.length; i++)
			{
				KatanaError* error = (KatanaError*)errors.data[i];
				std::cout << "Failed to parse css file \"" + path + "\"." << std::endl;
				std::cout << "Error is " << error->message << std::endl;
				std::cout << "ERROR at line " + std::to_string(error->first_line) +
					" : " + std::to_string(error->first_column) << std::endl;
			}

			return;
		}

		ParseRules(data->stylesheet, styleSheet);
	}

	StyleProperties GetPropFromString(const std::string& prop)
	{
		if (prop == "height")					return StyleProperties::Height;
		else if (prop == "max-height")			return StyleProperties::MaxHeight;
		else if (prop == "min-height")			return StyleProperties::MinHeight;
		else if (prop == "width")				return StyleProperties::Width;
		else if (prop == "max-width")			return StyleProperties::MaxWidth;
		else if (prop == "min-width")			return StyleProperties::MinWidth;
		else if (prop == "padding-left")		return StyleProperties::PaddingLeft;
		else if (prop == "padding-right")		return StyleProperties::PaddingRight;
		else if (prop == "padding-top")			return StyleProperties::PaddingTop;
		else if (prop == "padding-bottom")		return StyleProperties::PaddingBottom;
		else if (prop == "margin-left")			return StyleProperties::MarginLeft;
		else if (prop == "margin-right")		return StyleProperties::MarginRight;
		else if (prop == "margin-top")			return StyleProperties::MarginTop;
		else if (prop == "margin-bottom")		return StyleProperties::MarginBottom;
		else if (prop == "position")			return StyleProperties::Position;
		else if (prop == "align-items")			return StyleProperties::AlignItems;
		else if (prop == "self-align")			return StyleProperties::SelfAlign;
		else if (prop == "aspect-ratio")		return StyleProperties::AspectRatio;
		else if (prop == "flex-direction")		return StyleProperties::FlexDirection;
		else if (prop == "flex-wrap")			return StyleProperties::FlexWrap;
		else if (prop == "flex-basis")			return StyleProperties::FlexBasis;
		else if (prop == "flex-grow")			return StyleProperties::FlexGrow;
		else if (prop == "flex-shrink")			return StyleProperties::FlexShrink;
		else if (prop == "justify-content")		return StyleProperties::JustifyContent;
		else if (prop == "align-content")		return StyleProperties::AlignContent;
		else if (prop == "layout-direction")	return StyleProperties::LayoutDirection;
		else if (prop == "border-size")			return StyleProperties::BorderSize;
		else if (prop == "border-radius")		return StyleProperties::BorderRadius;
		else if (prop == "border-color")		return StyleProperties::BorderColor;
		else if (prop == "background-color")	return StyleProperties::BackgroundColor;
		else if (prop == "text-align")			return StyleProperties::TextAlign;
		else if (prop == "color")				return StyleProperties::Color;
		else if (prop == "overflow")			return StyleProperties::Overflow;
		else if (prop == "font-size")			return StyleProperties::FontSize;
		else if (prop == "visibility")			return StyleProperties::Visibility;
		else if (prop == "z-index")				return StyleProperties::ZIndex;
		else if (prop == "top")					return StyleProperties::Top;
		else if (prop == "bottom")				return StyleProperties::Bottom;
		else if (prop == "left")				return StyleProperties::Left;
		else if (prop == "right")				return StyleProperties::Right;
		else if (prop == "background-image")	return StyleProperties::BackgroundImage;
		return StyleProperties::None;
	}

	void StyleSheetParser::ParseStyleRule(KatanaRule* rule, StyleSheetPtr styleSheet)
	{
		auto styleRule = reinterpret_cast<KatanaStyleRule*>(rule);
		std::string styleName = rule->name;
		
		for (uint32_t s = 0; s < styleRule->selectors->length; s++)
		{
			auto styleSelector = std::vector<StyleSelector>();

			// unsafe c-style void* in the array.
			void* selectorData = styleRule->selectors->data[s];
			auto selector = reinterpret_cast<KatanaSelector*>(selectorData);
			while (selector)
			{
				auto match = selector->match; // tag, id or class
				switch (match)
				{
					case KatanaSelectorMatchPseudoClass:
					{
						std::string matchPseudo = selector->data->value;
						styleSelector.push_back({ StyleSelectorType::Pseudo, matchPseudo });
					}
					break;
					case KatanaSelectorMatchTag:
					{
						std::string matchTag = selector->tag->local;
						styleSelector.push_back({ StyleSelectorType::Tag, matchTag });
					}
					break;
					case KatanaSelectorMatchId:
					{
						std::string matchId = selector->data->value;
						styleSelector.push_back({ StyleSelectorType::Id, matchId });
					}
					break;
					case KatanaSelectorMatchClass:
					{
						std::string matchClass = selector->data->value;
						styleSelector.push_back({ StyleSelectorType::Class, matchClass });
					}
					break;
				}

				selector = selector->tagHistory;
			}

			// Added the new rule with selectors.
			auto newRule = StyleRule(styleSelector);

			// Now add the properties to the new rule.
			for (uint32_t d = 0; d < styleRule->declarations->length; d++)
			{
				// unsafe c-style void* in the array.
				void* declarationData = styleRule->declarations->data[d];
				auto declaration = reinterpret_cast<KatanaDeclaration*>(declarationData);

				// convert from string to property enum.
				StyleProperties propType = GetPropFromString(declaration->property);

				PropValue propValue{};
				for (uint32_t v = 0; v < declaration->values->length; v++)
				{
					// unsafe c-style voir* in the array.
					void* valueData = declaration->values->data[v];
					KatanaValue* value = reinterpret_cast<KatanaValue*>(valueData);

					switch (value->unit)
					{
					case KatanaValueUnit::KATANA_VALUE_STRING:
						{
							std::string stringValue = value->string;
							if (propType == StyleProperties::BackgroundImage)
							{
								stringValue = _parsingPath + "/../" + stringValue;
							}
							propValue.string = stringValue;
							propValue.type = PropValueType::String;
						}
						break;
						case KatanaValueUnit::KATANA_VALUE_PERCENTAGE:
						case KatanaValueUnit::KATANA_VALUE_PX:
						{
							propValue.value.Number = (float)value->fValue;
							propValue.type = value->unit == KatanaValueUnit::KATANA_VALUE_PX ? PropValueType::Pixel : PropValueType::Percent;
						}
						break;
						case KatanaValueUnit::KATANA_VALUE_PARSER_HEXCOLOR:
						{
							int r, g, b, a = 255;
							int result = sscanf_s(value->string, "%02x%02x%02x%02x", &r, &g, &b, &a);
							propValue.value.Color = Color(r, g, b, a);
							propValue.type = PropValueType::Color;
						}
						break;
						case KatanaValueUnit::KATANA_VALUE_UNKNOWN:
						{
							std::string valueStr = value->string;
						}
						break;
						case KatanaValueUnit::KATANA_VALUE_NUMBER:
							propValue.value.Number = (int)value->fValue;
							break;
						case KatanaValueUnit::KATANA_VALUE_IDENT:
						{
							std::string valueStr = value->string;
							if (propType == StyleProperties::Position)
							{
								PositionType positionType = PositionType::Relative;
								if (valueStr == "absolute")	
									propValue.value.Enum = (int)PositionType::Absolute;
							}
							if (propType == StyleProperties::AlignContent)
							{
								AlignContentType align;
								if (valueStr == "flex-start")			align = AlignContentType::FlexStart;
								else if (valueStr == "center")			align = AlignContentType::Center;
								else if (valueStr == "flex-end")		align = AlignContentType::FlexEnd;
								else if (valueStr == "stretch")			align = AlignContentType::Stretch;
								else if (valueStr == "space-between")	align = AlignContentType::SpaceBetween;
								else if (valueStr == "space-around")	align = AlignContentType::SpaceAround;
								else									align = AlignContentType::FlexStart;

								propValue.type = PropValueType::Enum;
								propValue.value.Enum = (int)align;
							}
							else if (propType == StyleProperties::AlignItems || propType == StyleProperties::SelfAlign)
							{
								AlignItemsType align;
								if (valueStr == "flex-start")			align = AlignItemsType::FlexStart;
								else if (valueStr == "center")			align = AlignItemsType::Center;
								else if (valueStr == "flex-end")		align = AlignItemsType::FlexEnd;
								else if (valueStr == "stretch")			align = AlignItemsType::Stretch;
								else if (valueStr == "space-between")	align = AlignItemsType::SpaceBetween;
								else if (valueStr == "space-around")	align = AlignItemsType::SpaceAround;
								else									align = AlignItemsType::FlexStart;

								propValue.type = PropValueType::Enum;
								propValue.value.Enum = (int)align;
							}
							else if (propType == StyleProperties::FlexDirection)
							{
								FlexDirectionType direction = FlexDirectionType::Row;
								if (valueStr == "column")				direction = FlexDirectionType::Column;
								else if (valueStr == "row-reversed")	direction = FlexDirectionType::RowReversed;
								else if (valueStr == "column-reversed")	direction = FlexDirectionType::ColumnReversed;
								
								propValue.type = PropValueType::Enum;
								propValue.value.Enum = (int)direction;
							}
							else if (propType == StyleProperties::FlexWrap)
							{
								FlexWrapType type = FlexWrapType::Wrap;
								if (valueStr == "no-wrap")	type = FlexWrapType::NoWrap;
								else if (valueStr == "wrap-reversed")	type = FlexWrapType::WrapReversed;

								propValue.type = PropValueType::Enum;
								propValue.value.Enum = (int)type;
							}
							else if (propType == StyleProperties::JustifyContent)
							{
								auto justify = JustifyContentType::FlexStart;
								if (valueStr == "center")			justify = JustifyContentType::Center;
								else if (valueStr == "flex-end")		justify = JustifyContentType::FlexEnd;
								else if (valueStr == "space-around")		justify = JustifyContentType::SpaceAround;
								else if (valueStr == "space-between")		justify = JustifyContentType::SpaceBetween;
								else if (valueStr == "space-evenly")		justify = JustifyContentType::SpaceEvenly;
								propValue.type = PropValueType::Enum;
								propValue.value.Enum = (int)justify;
							}
							else if (propType == StyleProperties::LayoutDirection)
							{
								auto direction = LayoutDirectionType::LTR;
								if (valueStr == "RTL")	direction = LayoutDirectionType::RTL;
								propValue.type = PropValueType::Enum;
								propValue.value.Enum = (int)direction;
							}
							else if (propType == StyleProperties::TextAlign)
							{
								auto align = TextAlignType::Left;
								if (valueStr == "center")	align = TextAlignType::Center;
								if (valueStr == "right")	align = TextAlignType::Right;
								propValue.type = PropValueType::Enum;
								propValue.value.Enum = (int)align;
							}
							else if (propType == StyleProperties::Overflow)
							{
								OverflowType overflow = OverflowType::Show;
								if (valueStr == "hidden") overflow = OverflowType::Hidden;
								else if (valueStr == "show") overflow = OverflowType::Show;
								else if (valueStr == "scroll") overflow = OverflowType::Scroll;

								propValue.type = PropValueType::Enum;
								propValue.value.Enum = (int)overflow;
							}
							else if (propType == StyleProperties::Visibility)
							{
								VisibilityType visibility = VisibilityType::Show;
								if (valueStr == "hidden") visibility = VisibilityType::Hidden;

								propValue.type = PropValueType::Enum;
								propValue.value.Enum = (int)visibility;
							}
						}
						break;
					}
				}

				newRule.SetProp(propType, propValue);
			}

			styleSheet->Rules.push_back(newRule);
		}
	}
}
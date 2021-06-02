#include "StyleSheetParser.h"
#include <regex>
#include <src/Vendors/pugixml/pugixml.hpp>

PropValue StyleSheetParser::ParsePropType(const std::string& str, PropType type)
{
	switch (type)
	{
	case PropType::HEIGHT: case PropType::MIN_HEIGHT:	case PropType::MAX_HEIGHT:
	case PropType::WIDTH: case PropType::MIN_WIDTH: case PropType::MAX_WIDTH:
	case PropType::LEFT:    case PropType::RIGHT:  case PropType::TOP:   case PropType::BOTTOM:
	case PropType::MARGIN_LEFT:  case PropType::MARGIN_RIGHT:  case PropType::MARGIN_TOP:  case PropType::MARGIN_BOTTOM:
	case PropType::PADDING_LEFT: case PropType::PADDING_RIGHT: case PropType::PADDING_TOP: case PropType::PADDING_BOTTOM:
	case PropType::BORDER_LEFT:  case PropType::BORDER_RIGHT:  case PropType::BORDER_TOP:  case PropType::BORDER_BOTTOM:
	{
		std::regex only_number("[0-9]+");
		std::regex not_number("[^0-9]+");
		// is percentage
		std::smatch match_value;
		if (std::regex_search(str.begin(), str.end(), match_value, only_number))
		{
			std::smatch match_type;
			if (std::regex_search(str.begin(), str.end(), match_type, not_number))
			{
				float value = std::stof(match_value[0]);
				if (match_type[0] == "%")
				{
					PropValue returnValue = PropValue{
						PropValueType::PERCENT,
						{0}
					};
					returnValue.value.Number = value;
					return returnValue;
				}
				else if (match_type[0] == "px")
				{
					PropValue returnValue = PropValue{
						PropValueType::PIXEL,
						{0}
					};
					returnValue.value.Number = value;
					return returnValue;
				}
			}
		}
	}
	break;

	}
	
	
	return PropValue();
}

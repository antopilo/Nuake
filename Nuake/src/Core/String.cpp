#include "String.h"
#include <iostream>
#include <sstream>

namespace Nuake 
{
	bool String::BeginsWith(const std::string& string, const std::string& begin)
	{
		if (string.rfind(begin, 0) == 0)
		{
			return true;
		}

		return false;
	}

	bool String::EndsWith(const std::string& string, const std::string& end)
	{
		if (string.length() >= end.length())
		{
			return (0 == string.compare(string.length() - end.length(), end.length(), end));
		}

		return false;
	}

	std::vector<std::string> String::Split(const std::string& string, char delimiter)
	{
		std::vector<std::string> result;
		std::stringstream ss(string);
		std::string item;

		while (getline(ss, item, delimiter))
		{
			result.push_back(item);
		}

		return result;
	}

	float String::ToFloat(const std::string& string)
	{
		return std::stof(string);
	}
}
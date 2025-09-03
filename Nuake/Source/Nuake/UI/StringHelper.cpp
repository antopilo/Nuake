#include "StringHelper.h"

#include <sstream>
#include <iostream>

namespace NuakeUI::StringHelper
{
	std::vector<std::string> Split(std::string string, const char delimiter)
	{
		auto splittedString = std::vector<std::string>();

		std::stringstream ss(string);
		std::string element;

		while (getline(ss, element, delimiter))
		{
			splittedString.push_back(element);
		}

		return splittedString;
	}

	std::string RemoveChar(std::string string, char character)
	{
		// remove space from string
		string.erase(std::remove(string.begin(), string.end(), character), string.end());
		return string;
	}

	std::vector<std::string> Split(std::string s, const std::string& delimiter)
	{
		auto splittedString = std::vector<std::string>();

		std::vector<std::string> splits;
		size_t pos = 0;
		std::string token;
		while ((pos = s.find(delimiter)) != std::string::npos) {
			token = s.substr(0, pos);
			splits.push_back(token);
			s.erase(0, pos + delimiter.length());
		}
		splits.push_back(s);

		return splits;
	}

	bool StartsWith(const std::string& str, const std::string& start)
	{
		return str.size() >= start.size() && 0 == str.compare(0, start.size(), start);
	}

	bool EndsWith(const std::string& str, const std::string& end)
	{
		return str.size() >= end.size() && 0 == str.compare(str.size() - end.size(), end.size(), end);
	}
}
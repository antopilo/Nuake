#include "String.h"
#include <iostream>
#include <regex>
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

	bool String::IsDigit(const char& character)
	{
		return character <= 57 && character >= 48;
	}

	std::string String::RemoveWhiteSpace(const std::string& string)
	{
		std::string result = string;
		result.erase(remove_if(result.begin(), result.end(), isspace), result.end());
		return result;
	}

	std::string String::Sanitize(const std::string& keyword) {
		std::string sanitizedKeyword = keyword;

		// Remove spaces, underscores, and hyphens using regex
		sanitizedKeyword = std::regex_replace(sanitizedKeyword, std::regex("[ _-]+"), "");

		// Convert to lowercase
		std::transform(sanitizedKeyword.begin(), sanitizedKeyword.end(), sanitizedKeyword.begin(), ::tolower);

		return sanitizedKeyword;
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

	std::string String::ToUpper(const std::string& string)
	{
		if(string.length() == 0)
		{
			return "";
		}

		auto split = Split(string, ' ');
		std::string result;
		for (auto& word : split)
		{
			word[0] = std::toupper(word[0]);
			result += word;
		}

		return result;
	}
}
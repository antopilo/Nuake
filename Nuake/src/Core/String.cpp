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

	bool String::EndsWith(const std::string& string, const std::string_view& end)
	{
		if (string.length() >= end.length())
		{
			bool result = string.ends_with(end);

			return result;
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

	std::string String::Sanitize(const std::string& keyword)
	{
		std::string sanitizedKeyword = keyword;

		// Remove spaces, underscores, and hyphens using regex
		sanitizedKeyword = std::regex_replace(sanitizedKeyword, std::regex("[ _-]+"), "");

		// Convert to lowercase
		//std::transform(sanitizedKeyword.begin(), sanitizedKeyword.end(), sanitizedKeyword.begin(), ::tolower);

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

	std::string String::ToLower(const std::string& str) 
	{
		std::string result = str; // Create a copy of the input string

		// Transform each character to lowercase
		std::transform(result.begin(), result.end(), result.begin(),
					   [](unsigned char c) { return std::tolower(c); });

		return result;
	}

	std::string String::ReplaceSlash(const std::string& str)
	{
		std::string result = str;
		std::replace(result.begin(), result.end(), '\\', '/');
		return result;
	}

	std::string String::Base64Encode(const std::vector<uint8_t>& data)
	{
		static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		std::string result;
		int val = 0, valb = -6;
		for (uint8_t c : data) 
		{
			val = (val << 8) + c;
			valb += 8;
			while (valb >= 0) 
			{
				result.push_back(chars[(val >> valb) & 0x3F]);
				valb -= 6;
			}
		}
		if (valb > -6) result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
		while (result.size() % 4) result.push_back('=');
		return result;
	}

	std::vector<uint8_t> String::Base64Decode(const std::string& data)
	{
		static const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		std::vector<uint8_t> out;
		std::vector<int> T(256, -1);
		for (int i = 0; i < 64; i++) T[chars[i]] = i;
		int val = 0, valb = -8;
		for (unsigned char c : data) 
		{
			if (T[c] == -1) break;
			val = (val << 6) + T[c];
			valb += 6;
			if (valb >= 0) 
			{
				out.push_back(uint8_t((val >> valb) & 0xFF));
				valb -= 8;
			}
		}
		return out;
	}
}
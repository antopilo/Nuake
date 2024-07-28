#pragma once
#include <string>
#include <vector>

/*
	This class purpose is to provide helper methods related to strings.
	It is not meant to be a replacement for std::string.
*/
namespace Nuake 
{
	class String 
	{
	public:
		static bool BeginsWith(const std::string& string, const std::string& begin);
		static bool EndsWith(const std::string& string, const std::string_view& end);
		static bool IsDigit(const char& character);
		static std::string RemoveWhiteSpace(const std::string& string);
		static std::string Sanitize(const std::string& keyword);
		static std::vector<std::string> Split(const std::string& string, char delimiter);

		static float ToFloat(const std::string& string);
		static std::string ToUpper(const std::string& string);
		static std::string ToLower(const std::string& string);
	};
}

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
		static bool EndsWith(const std::string& string, const std::string& end);
		static std::vector<std::string> Split(const std::string& string, char delimiter);

		static float ToFloat(const std::string& string);
	};
}

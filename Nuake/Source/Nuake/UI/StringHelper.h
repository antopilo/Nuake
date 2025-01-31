#pragma once
#include <string>
#include <vector>

namespace NuakeUI
{
	namespace StringHelper
	{
		/// <summary>
		/// Splits a string into a vector of substrings.
		/// </summary>
		/// <param name="string">The string to be split</param>
		/// <param name="delimiter">The token</param>
		/// <returns></returns>
		std::vector<std::string> Split(std::string string, const char delimiter);

		std::string RemoveChar(std::string string, char character);

		std::vector<std::string> Split(std::string s, const std::string& delimiter);

		bool StartsWith(const std::string& str, const std::string& start);

		bool EndsWith(const std::string& str, const std::string& end);
	}
}
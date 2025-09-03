#pragma once
#include <string>


class File
{
	File(const std::string& path);
	void Write(const std::string& content);
};
#pragma once
#include <string>
class FGDSerializer
{
	static bool BeginFGDFile(const std::string path);

	static bool RegisterEntity();

	static bool EndFGDFile();
};
#pragma once
#include <string>
class FGDSerializer
{
public:
	static bool BeginFGDFile(const std::string path);

	static bool RegisterEntity();

	static bool EndFGDFile();
};
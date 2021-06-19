#include "FDGSerializer.h"
#include <src\Core\FileSystem.h>


bool FGDSerializer::BeginFGDFile(const std::string path)
{
	FileSystem::BeginWriteFile(path + ".fgd");
}
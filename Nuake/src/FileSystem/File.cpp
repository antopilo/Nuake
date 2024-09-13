#include "File.h"

#include "src/FileSystem/FileSystem.h"


using namespace Nuake;

File::File(Ref<Directory> parentDir, const std::string& fullPath, const std::string& fileName, const std::string& fileType) :
	name(fileName),
	type(fileType),
	absolutePath(fullPath),
	relativePath(FileSystem::AbsoluteToRelative(fullPath)),
	modified(false)
{ }

FileType File::GetFileType() const
{
	const std::string ext = GetExtension();
	if (ext == ".png" || ext == ".jpg")
	{
		return FileType::Image;
	}
	if (ext == ".ogg" || ext == ".wav")
	{
		return FileType::Audio;
	}
	if (ext == ".material")
	{
		return FileType::Material;
	}

	if (ext == ".scene")
	{
		return FileType::Scene;
	}

	if (ext == ".project")
	{
		return FileType::Project;
	}

	if (ext == ".script")
	{
		return FileType::Script;
	}

	if (ext == ".prefab")
	{
		return FileType::Prefab;
	}

	if (ext == ".wad")
	{
		return FileType::Wad;
	}

	if (ext == ".map")
	{
		return FileType::Map;
	}

	if (ext == ".dll")
	{
		return FileType::Assembly;
	}

	if (ext == ".cs")
	{
		return FileType::NetScript;
	}

	if (ext == ".sln")
	{
		return FileType::Solution;
	}

	if (ext == ".mesh")
	{
		return FileType::Mesh;
	}

	if (ext == ".html")
	{
		return FileType::UI;
	}

	if (ext == ".css")
	{
		return FileType::CSS;
	}

	return FileType::Unkown;
}

std::string File::GetFileTypeAsString() const
{
	std::string ext = GetExtension();
	if (ext == ".png" || ext == "jpg")
	{
		return "Image";
	}

	if (ext == ".material")
	{
		return "Material";
	}

	if (ext == ".scene")
	{
		return "Scene";
	}

	if (ext == ".project")
	{
		return "Project";
	}

	if (ext == ".script")
	{
		return "Script";
	}

	if (ext == ".prefab")
	{
		return "Prefab";
	}

	if (ext == ".wad")
	{
		return "Wad";
	}

	if (ext == ".map")
	{
		return "Map";
	}

	if (ext == ".map")
	{
		return "Assembly";
	}

	if (ext == ".cs")
	{
		return "C# Script";
	}

	if (ext == ".html")
	{
		return "UI Layout";
	}

	if (ext == ".css")
	{
		return "StyleSheet";
	}

	return "File";
}

std::string File::Read() const
{
	return FileSystem::ReadFile(absolutePath, true);
}

bool File::Exist() const
{
	return FileSystem::FileExists(absolutePath, true);
}
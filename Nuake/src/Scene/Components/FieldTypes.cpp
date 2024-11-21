#include "FieldTypes.h"

#include "src/FileSystem/File.h"

bool Nuake::ResourceFile::Exist()
{
    return file != nullptr && file->Exist();
}

std::string Nuake::ResourceFile::GetRelativePath()
{
    if (Exist())
    {
        return file->GetRelativePath();
    }

    return "";
}

std::string Nuake::ResourceFile::GetAbsolutePath()
{
    if (Exist())
    {
        return file->GetAbsolutePath();
    }

    return "";
}

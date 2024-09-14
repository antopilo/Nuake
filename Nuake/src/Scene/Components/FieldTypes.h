#pragma once

#include <string>

#include "src/Core/Core.h"

namespace Nuake
{
    class File;
    
    struct ResourceFile
    {
        ResourceFile() = default;
        ResourceFile(const Ref<File>& inFile) : file(inFile) {}

        bool Exist();
        std::string GetRelativePath();
        std::string GetAbsolutePath();
        
        Ref<File> file = nullptr;
    };

    struct DynamicItemList
    {
        std::vector<std::string> items;
        int index = -1;
    };
}

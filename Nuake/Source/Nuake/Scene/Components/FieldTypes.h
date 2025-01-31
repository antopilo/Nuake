#pragma once

#include <string>

#include "Nuake/Core/Core.h"

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
        bool dirty = true;
    };

    struct DynamicItemList
    {
        std::vector<std::string> items;
        int index = -1;
    };
}

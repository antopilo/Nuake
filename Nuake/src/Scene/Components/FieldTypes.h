#pragma once

#include <string>

#include "src/Core/Core.h"

namespace Nuake
{
    class File;
    
    struct ResourceFile
    {
        ResourceFile() {}
        ResourceFile(const Ref<File>& inFile) : file(inFile) {}
        
        Ref<File> file = nullptr;
    };
}

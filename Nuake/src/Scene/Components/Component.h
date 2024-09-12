#pragma once

#include "src/Core/Object/Object.h"

namespace Nuake
{
    class Component
    {
    protected:
        static void InitializeComponentClass() {}

        static void (* GetInitializeComponentClass(void))()
        {
            return &Component::InitializeComponentClass;
        }
        
    public:
        static std::string GetName(const entt::meta_type& componentMeta);
    };
}

#pragma once

namespace Nuake
{
    class ClassDB
    {
    public:
        template<class T>
        static void RegisterComponent(T klass)
        {
            T::InternalInitializeClass();
        }
    };
}

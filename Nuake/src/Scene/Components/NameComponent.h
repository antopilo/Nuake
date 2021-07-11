#pragma once
#include "src/Resource/Serializable.h"
#include "src/Core/OS.h"

namespace Nuake {
    class NameComponent 
    {
    public:
        std::string Name = "Entity";
        int ID;

        json Serialize()
        {
            BEGIN_SERIALIZE();
            SERIALIZE_VAL(Name);
            SERIALIZE_VAL(ID);
            END_SERIALIZE();
        }

        bool Deserialize(const std::string& str)
        {
            BEGIN_DESERIALIZE();
            Name = j["Name"];

            if (j.contains("ID"))
                ID = j["ID"];
            else
                ID = OS::GetTime();

            return true;
        }
    };
}

#pragma once
#include "src/Core/OS.h"
#include "src/Resource/Serializable.h"

namespace Nuake {
    class NameComponent 
    {
    public:
		int ID;
        std::string Name = "Entity";
		bool IsPrefab = false;

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

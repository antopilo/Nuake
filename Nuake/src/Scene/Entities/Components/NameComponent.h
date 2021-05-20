#pragma once
#include "../Resource/Serializable.h"

class NameComponent {
public:
    std::string Name = "Entity";

    json Serialize()
    {
        BEGIN_SERIALIZE();
            SERIALIZE_VAL(Name);
        END_SERIALIZE();
    }

    bool Deserialize(const std::string& str)
    {
        BEGIN_DESERIALIZE();
        Name = j["Name"];
        return true;
    }
};
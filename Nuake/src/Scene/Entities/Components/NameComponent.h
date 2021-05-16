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
};
#pragma once
#include "src/Core/Core.h"
#include "src/UI/UserInterface.h"
#include "src/Resource/Serializable.h"

namespace Nuake {
	class InterfaceComponent
	{
    public:
        std::string Path;
		Ref<UI::UserInterface> Interface;

        json Serialize()
        {
            BEGIN_SERIALIZE();
            SERIALIZE_VAL_LBL("Interface", Interface->GetPath())
            END_SERIALIZE();
        }

        bool Deserialize(const std::string& str)
        {
            BEGIN_DESERIALIZE();

            if (j.contains("Interface"))
                SetInterface(j["Interface"]);

            return true;
        }

        void SetInterface(const std::string& path)
        {
            Path = path;
            Interface = UI::UserInterface::New("No name", path);
        }
	};
}

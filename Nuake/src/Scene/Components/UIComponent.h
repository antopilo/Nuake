#pragma once
#include "src/Core/Object/Object.h"
#include "src/Resource/UUID.h"
#include "src/Resource/Serializable.h"

namespace Nuake
{
	struct UIComponent
	{
		NUAKECOMPONENT(UIComponent, "UI Component");

		UUID UIResource = UUID(0);
		std::string CSharpUIController;
		std::string UIFilePath;
		bool IsWorldSpace;
		// TODO: Z-Ordering

		json Serialize()
		{
			BEGIN_SERIALIZE();
			SERIALIZE_VAL(UIFilePath);
			SERIALIZE_VAL(IsWorldSpace);
			END_SERIALIZE();
		}

		bool Deserialize(const json& j)
		{
			DESERIALIZE_VAL(UIFilePath);
			DESERIALIZE_VAL(IsWorldSpace);
			return true;
		}
	};
}
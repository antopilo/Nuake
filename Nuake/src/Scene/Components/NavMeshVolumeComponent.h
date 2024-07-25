#pragma once
#include "../Entities/Entity.h"
#include "Engine.h"

namespace Nuake
{
	struct NavMeshVolumeComponent
	{
		Vector3 VolumeSize = { 1.0f, 1.0f, 1.0f };
		bool OnlyIncludeMapGeometry = true;

		json Serialize()
		{
			BEGIN_SERIALIZE();
			SERIALIZE_VEC3(VolumeSize);
			SERIALIZE_VAL(OnlyIncludeMapGeometry);
			END_SERIALIZE();
		}

		bool Deserialize(const json& j)
		{
			if (j.contains("VolumeSize"));
			{
				DESERIALIZE_VEC3(j["VolumeSize"], VolumeSize);
			}

			if (j.contains("OnlyIncludeMapGeometry"))
			{
				DESERIALIZE_VAL(OnlyIncludeMapGeometry);
			}

			return true;
		}
	};
}

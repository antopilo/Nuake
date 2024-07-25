#pragma once
#include "../Entities/Entity.h"
#include "Engine.h"

namespace Nuake
{
	struct NavMeshVolumeComponent
	{
		Vector3 VolumeSize = { 1.0f, 1.0f, 1.0f };
		bool OnlyIncludeMapGeometry = true;

		// Generation config
		float CellSize = 0.2f;
		float CellHeight = 0.2f;
		float TileSize = 10.0f;
		float WalkableSlopeAngle = 45.0f;
		float MaxEdgeLength = 12.0f;
		float MaxVertsPerPoly = 6.0f;
		float WalkableHeight = 1.0f;
		float WalkableClimb = 1.0f;
		float WalkableRadius = 1.0f;
		float MaxSimplificationError = 1.3f;
		float MinRegionArea = 4.0f;
		float MergeRegionArea = 10.0f;
		float DetailSampleDistance = 2.0f;
		float DetailsampleMaxError = 1.0f;

		json Serialize()
		{
			BEGIN_SERIALIZE();
			SERIALIZE_VEC3(VolumeSize);
			SERIALIZE_VAL(OnlyIncludeMapGeometry);

			// Generation config
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

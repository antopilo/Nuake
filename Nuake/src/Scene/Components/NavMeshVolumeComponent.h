#pragma once
#include "../Entities/Entity.h"
#include "Engine.h"
#include <src/AI/NavMesh.h>

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

		float AgentHeight = 0.5f;
		float AgentRadius = 0.1f;
		float AgentMaxClimb = 0.01f;

		Ref<NavMesh> NavMeshData;

		json Serialize()
		{
			BEGIN_SERIALIZE();
			SERIALIZE_VEC3(VolumeSize);
			SERIALIZE_VAL(OnlyIncludeMapGeometry);

			SERIALIZE_VAL(CellSize);
			SERIALIZE_VAL(CellHeight);
			SERIALIZE_VAL(TileSize);
			SERIALIZE_VAL(WalkableSlopeAngle);
			SERIALIZE_VAL(MaxEdgeLength);
			SERIALIZE_VAL(MaxVertsPerPoly);
			SERIALIZE_VAL(WalkableHeight);
			SERIALIZE_VAL(WalkableClimb);
			SERIALIZE_VAL(WalkableRadius);
			SERIALIZE_VAL(MaxSimplificationError);
			SERIALIZE_VAL(MinRegionArea);
			SERIALIZE_VAL(DetailSampleDistance);
			SERIALIZE_VAL(DetailsampleMaxError);

			SERIALIZE_VAL(AgentHeight);
			SERIALIZE_VAL(AgentRadius);
			SERIALIZE_VAL(AgentMaxClimb);

			if (NavMeshData != nullptr)
			{
				SERIALIZE_OBJECT(NavMeshData);
			}

			// Generation config
			END_SERIALIZE();
		}

		bool Deserialize(const json& j)
		{
			if (j.contains("VolumeSize"))
			{
				DESERIALIZE_VEC3(j["VolumeSize"], VolumeSize);
			}

			if (j.contains("OnlyIncludeMapGeometry"))
			{
				DESERIALIZE_VAL(OnlyIncludeMapGeometry);
			}

			DESERIALIZE_VAL(CellSize);
			DESERIALIZE_VAL(CellHeight);
			DESERIALIZE_VAL(TileSize);
			DESERIALIZE_VAL(WalkableSlopeAngle);
			DESERIALIZE_VAL(MaxEdgeLength);
			DESERIALIZE_VAL(MaxVertsPerPoly);
			DESERIALIZE_VAL(WalkableHeight);
			DESERIALIZE_VAL(WalkableClimb);
			DESERIALIZE_VAL(WalkableRadius);
			DESERIALIZE_VAL(MaxSimplificationError);
			DESERIALIZE_VAL(MinRegionArea);
			DESERIALIZE_VAL(DetailSampleDistance);
			DESERIALIZE_VAL(DetailsampleMaxError);
			DESERIALIZE_VAL(AgentHeight);
			DESERIALIZE_VAL(AgentRadius);
			DESERIALIZE_VAL(AgentMaxClimb);

			if (j.contains("NavMeshData"))
			{
				NavMeshData = CreateRef<NavMesh>();
				NavMeshData->Deserialize(j["NavMeshData"]);
			}

			return true;
		}
	};
}

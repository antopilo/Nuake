#pragma once
#include "src/Core/Maths.h"

namespace Nuake {

	struct NavMeshVolumeComponent;

	class RecastConfig
	{
	public:
		Vector3 Bound = Vector3(100, 100, 100);
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

		RecastConfig(const NavMeshVolumeComponent& component);
		RecastConfig() = default;
		
		~RecastConfig() = default;
	};
}
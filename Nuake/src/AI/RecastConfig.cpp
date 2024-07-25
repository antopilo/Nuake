#include "RecastConfig.h"
#include "src/Scene/Components/NavMeshVolumeComponent.h"


namespace Nuake {

	RecastConfig::RecastConfig(const NavMeshVolumeComponent& component)
	{
		Bound = component.VolumeSize;
		CellSize = component.CellSize;
		CellHeight = component.CellHeight;
		TileSize = component.TileSize;
		WalkableSlopeAngle = component.WalkableSlopeAngle;
		MaxEdgeLength = component.MaxEdgeLength;
		MaxVertsPerPoly = component.MaxVertsPerPoly;
		WalkableHeight = component.WalkableHeight;
		WalkableClimb = component.WalkableClimb;
		WalkableRadius = component.WalkableRadius;
		MaxSimplificationError = component.MaxSimplificationError;
		MinRegionArea = component.MinRegionArea;
		MergeRegionArea = component.MergeRegionArea;
		DetailSampleDistance = component.DetailSampleDistance;
		DetailsampleMaxError = component.DetailsampleMaxError;
	}
}
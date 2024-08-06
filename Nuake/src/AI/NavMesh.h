#pragma once
#include <src/Core/Maths.h>
#include "src/Resource/Serializable.h"
#include <DetourNavMeshQuery.h>
#include <DetourNavMesh.h>

namespace Nuake {

	class NavMesh : ISerializable {

	public:
		NavMesh(dtNavMesh* navMesh, dtNavMeshQuery* navMeshQuery);
		NavMesh() = default;
		~NavMesh();

		// Query
		int FindNearestPolygon(const Vector3 & searchPosition, const Vector3 & searchBox);

		std::vector<Vector3> FindPath(int polyStart, int polyEnd, const Vector3& nearestPointStart, const Vector3& nearestPointEnd, const int maxPoly);

		json Serialize() override;
		bool Deserialize(const json& j) override;

	private:
		const dtMeshTile* GetTile(int i) const;

		dtNavMesh* m_DetourNavMesh;
		dtNavMeshQuery* m_DetourNavQuery;
	};
}
#pragma once
#include <src/Core/Maths.h>
#include "src/Resource/Serializable.h"
#include <DetourNavMeshQuery.h>
#include <DetourNavMesh.h>

namespace Nuake {

	class NavMesh : ISerializable 
	{
	private:
		struct PolySearchResult
		{
			int PolyID;
			Vector3 PositionInPoly;
		};

	public:
		NavMesh(dtNavMesh* navMesh, dtNavMeshQuery* navMeshQuery);
		NavMesh() = default;
		~NavMesh();

		// Query
		PolySearchResult FindNearestPolygon(const Vector3 & searchPosition, const Vector3 & searchBox);

		std::vector<Vector3> FindStraightPath(const Vector3& start, const Vector3& end);

		std::vector<Vector3> FindPath(int polyStart, int polyEnd, const Vector3& nearestPointStart, const Vector3& nearestPointEnd, const int maxPoly);

		json Serialize() override;
		bool Deserialize(const json& j) override;

		bool IsValid() const 
		{
			return m_DetourNavMesh != nullptr && m_DetourNavQuery != nullptr;
		}

		dtNavMesh* GetNavMesh() const { return m_DetourNavMesh; }

	private:
		const dtMeshTile* GetTile(int i) const;

		dtNavMesh* m_DetourNavMesh;
		dtNavMeshQuery* m_DetourNavQuery;
	};
}
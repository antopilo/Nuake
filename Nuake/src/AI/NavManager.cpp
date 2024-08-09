#include "NavManager.h"
#include "src/Core/Logger.h"
#include "src/Rendering/Vertex.h"
#include "Recast.h"
#include <DetourNavMeshBuilder.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>
#include <DetourDebugDraw.h>
#include <DebugDraw.h>

#include "Engine.h"
#include "src/Scene/Entities/Entity.h"
#include "src/Core/Maths.h"

#include "src/AI/RecastConfig.h"
#include "NavMesh.h"

namespace Nuake {

	void NavManager::Initialize()
	{
		m_RecastContext = CreateRef<rcContext>();
	}

	void NavManager::Cleanup()
	{
		dtFree(m_DetourNavMesh);
	}

	void NavManager::PushMesh(const Ref<Mesh>& mesh, const Matrix4& transform)
	{
		m_Meshes.push_back({ mesh, transform });
	}

	Ref<NavMesh> NavManager::BuildNavMesh(const RecastConfig& config)
	{
		// Merge all meshes togheter
		std::vector<Vector3> vertices;
		std::vector<int> indices;

		// Since we are merging all the meshes together, we need to offset
		// the indices so they point to the correct vertices in the array.
		// Example: Model1 + Model2 + Model3 will get merged together in one array
		// and we need to make sure the indices of Model2 point to the correct location in the merged array
		uint32_t currentVertexOffset = 0;
		for (auto& mesh : m_Meshes)
		{
			for (auto& vert : mesh.mesh->GetVertices())
			{
				Vector4 transformVertex = mesh.transform * Vector4(vert.position.x, vert.position.y, vert.position.z, 1.0f);
				vertices.push_back({ transformVertex.x, transformVertex.y, transformVertex.z });
			}

			for (auto& index : mesh.mesh->GetIndices())
			{
				indices.push_back(currentVertexOffset + index);
			}

			currentVertexOffset = std::size(vertices);
		}

		float bmin[3] = { config.Position.x - config.Bound.x, config.Position.y - config.Bound.y, config.Position.z - config.Bound.z };
		float bmax[3] = { config.Position.x + config.Bound.x, config.Position.y + config.Bound.y, config.Position.z + config.Bound.z };
		rcConfig recastConfig;
		recastConfig.cs = config.CellSize;
		recastConfig.ch = config.CellHeight;
		recastConfig.tileSize = config.TileSize;
		recastConfig.walkableSlopeAngle = config.WalkableSlopeAngle;
		recastConfig.maxEdgeLen = config.MaxEdgeLength;
		recastConfig.maxVertsPerPoly = config.MaxVertsPerPoly;
		recastConfig.walkableHeight = config.WalkableHeight;
		recastConfig.walkableClimb = config.WalkableClimb;
		recastConfig.walkableRadius = config.WalkableRadius;
		recastConfig.maxSimplificationError = config.MaxSimplificationError;
		recastConfig.minRegionArea = config.MinRegionArea;
		recastConfig.mergeRegionArea = config.MergeRegionArea;
		recastConfig.detailSampleDist = config.DetailSampleDistance;
		recastConfig.detailSampleMaxError = config.DetailsampleMaxError;
		
		rcVcopy(recastConfig.bmin, bmin);
		rcVcopy(recastConfig.bmax, bmax);

		// Calculate grid size given world size
		rcCalcGridSize(recastConfig.bmin, recastConfig.bmax, recastConfig.cs, &recastConfig.width, &recastConfig.height);

		Logger::Log("Building navigation:", "NavManager", VERBOSE);
		Logger::Log(" - " + std::to_string(recastConfig.width) + " x " + std::to_string(recastConfig.height), "NavManager", VERBOSE);

		auto voxelHeightField = rcAllocHeightfield();
		if (!voxelHeightField)
		{
			Logger::Log("buildNavigation: Out of memory 'solid'.", "NavManager", CRITICAL);
		}
		if (!rcCreateHeightfield(m_RecastContext.get(), *voxelHeightField, recastConfig.width, recastConfig.height, recastConfig.bmin, recastConfig.bmax, recastConfig.cs, recastConfig.ch))
		{
			Logger::Log("buildNavigation: Could not create solid heightfield.", "NavManager", CRITICAL);
		}

		unsigned char* m_triareas = new unsigned char[indices.size()];
		memset(m_triareas, 0, indices.size() * sizeof(unsigned char));

		float* verts = reinterpret_cast<float*>(vertices.data());
		int* tris = reinterpret_cast<int*>(indices.data());
		rcMarkWalkableTriangles(m_RecastContext.get(), 45.0f, verts, std::size(vertices), tris, std::size(indices) / 3, m_triareas);
		if (!rcRasterizeTriangles(m_RecastContext.get(), verts, std::size(vertices), tris, m_triareas, std::size(indices) / 3, *voxelHeightField, recastConfig.walkableClimb))
		{
			Logger::Log("buildNavigation: Could not rasterize triangles.", "NavManager", CRITICAL);
		}

		// Filter walkable surfaces.
		const bool filterLowHangingObstacles = true;
		const bool filterLedgeSpans = true;
		const bool filterWalkableLowHeightSpans = true;
		if (filterLowHangingObstacles)
		{
			rcFilterLowHangingWalkableObstacles(m_RecastContext.get(), recastConfig.walkableClimb, *voxelHeightField);
		}

		if (filterLedgeSpans)
		{
			rcFilterLedgeSpans(m_RecastContext.get(), recastConfig.walkableHeight, recastConfig.walkableClimb, *voxelHeightField);
		}

		if (filterWalkableLowHeightSpans)
		{
			rcFilterWalkableLowHeightSpans(m_RecastContext.get(), recastConfig.walkableHeight, *voxelHeightField);
		}

		auto compactHeightField = rcAllocCompactHeightfield();
		bool result = rcBuildCompactHeightfield(m_RecastContext.get(), recastConfig.walkableHeight, recastConfig.walkableClimb, *voxelHeightField, *compactHeightField);

		if (!result)
		{
			Logger::Log("buildNavigation: Could not build compact data.", "NavManager", CRITICAL);
		}

		// We dont need to keep in memory the height field
		rcFreeHeightField(voxelHeightField);

		result = rcErodeWalkableArea(m_RecastContext.get(), recastConfig.walkableRadius, *compactHeightField);
		if (!result)
		{
			Logger::Log("buildNavigation: Could not erode.", "NavManager", CRITICAL);
		}

		// Water shed for now...
		result = rcBuildDistanceField(m_RecastContext.get(), *compactHeightField);
		if (!result)
		{
			Logger::Log("buildNavigation: Could not build distance field.", "NavManager", CRITICAL);
		}

		result = rcBuildRegions(m_RecastContext.get(), *compactHeightField, 0, recastConfig.minRegionArea, recastConfig.mergeRegionArea);
		if (!result)
		{
			Logger::Log("buildNavigation: Could not build watershed regions.", "NavManager", CRITICAL);
		}

		// Trace and simplify region countours.
		auto contourSet = rcAllocContourSet();
		result = rcBuildContours(m_RecastContext.get(), *compactHeightField, recastConfig.maxSimplificationError, recastConfig.maxEdgeLen, *contourSet);
		if (!result)
		{
			Logger::Log("buildNavigation: Could not create contours.", "NavManager", CRITICAL);
		}

		// Build polygons mesh from contours
		auto polygonMesh = rcAllocPolyMesh();
		result = rcBuildPolyMesh(m_RecastContext.get(), *contourSet, recastConfig.maxVertsPerPoly, *polygonMesh);
		if (!result)
		{
			Logger::Log("buildNavigation: Could not triangulate contours.", "NavManager", CRITICAL);
		}

		// Create detail polygon mesh
		auto detailMesh = rcAllocPolyMeshDetail();
		result = rcBuildPolyMeshDetail(m_RecastContext.get(), *polygonMesh, *compactHeightField, recastConfig.detailSampleDist, recastConfig.detailSampleMaxError, *detailMesh);
		if (!result)
		{
			Logger::Log("buildNavigation: Could not build detail mesh.", "NavManager", CRITICAL);
		}

		// Free memory
		rcFreeCompactHeightfield(compactHeightField);
		rcFreeContourSet(contourSet);

		// Apply flags and areas type checking here, example, swim, grass, door, etc.
		// Should be user defined almost...

		// DETOUR 
		unsigned char* navData = 0;
		int navDataSize = 0;

		for (int i = 0; i < polygonMesh->npolys; i++)
		{
			if (polygonMesh->areas[i] == RC_WALKABLE_AREA)
			{
				polygonMesh->areas[i] = 0;
			}

			if (polygonMesh->areas[i] == 0)
			{
				polygonMesh->flags[i] = 1;
			}
		}

		// Create detour data from poly mesh...
		dtNavMeshCreateParams params;
		memset(&params, 0, sizeof(params));
		params.verts = polygonMesh->verts;
		params.vertCount = polygonMesh->nverts;
		params.polys = polygonMesh->polys;
		params.polyAreas = polygonMesh->areas;
		params.polyFlags = polygonMesh->flags;
		params.polyCount = polygonMesh->npolys;
		params.nvp = polygonMesh->nvp;
		params.detailVerts = detailMesh->verts;
		params.detailVertsCount = detailMesh->nverts;
		params.detailTris = detailMesh->tris;
		params.detailTriCount = detailMesh->ntris;

		// Off connection, might be useful for user defined jump points?
		// Maybe useful for ziplines, scripted jumppad, idk
		// See sample OffMeshConnectionTool.cpp in RecastDemo project.

		//params.offMeshConVerts = m_geom->getOffMeshConnectionVerts();
		//params.offMeshConRad = m_geom->getOffMeshConnectionRads();
		//params.offMeshConDir = m_geom->getOffMeshConnectionDirs();
		//params.offMeshConAreas = m_geom->getOffMeshConnectionAreas();
		//params.offMeshConFlags = m_geom->getOffMeshConnectionFlags();
		//params.offMeshConUserID = m_geom->getOffMeshConnectionId();
		//params.offMeshConCount = m_geom->getOffMeshConnectionCount();

		const float agentHeight = 0.5f;
		const float agentRadius = 0.1f;
		const float agentMaxClimb = 0.01f;
		params.walkableHeight = agentHeight;
		params.walkableRadius = agentRadius;
		params.walkableClimb = agentMaxClimb;

		rcVcopy(params.bmin, polygonMesh->bmin);
		rcVcopy(params.bmax, polygonMesh->bmax);

		// Cell size and cell height
		params.cs = recastConfig.cs;
		params.ch = recastConfig.ch;
		params.buildBvTree = true; // Not sure the difference it makes in performance. based on sample.

		result = dtCreateNavMeshData(&params, &navData, &navDataSize);
		if (!result)
		{
			Logger::Log("Could not build Detour navmesh.", "NavManager", CRITICAL);
		}

		m_DetourNavMesh = dtAllocNavMesh();
		if (!m_DetourNavMesh)
		{
			Logger::Log("Could not create Detour navmesh.", "NavManager", CRITICAL);
		}

		dtStatus status;
		status = m_DetourNavMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
		if (dtStatusFailed(status))
		{
			dtFree(navData);
			Logger::Log("Could not init Detour navmesh", "NavManager", CRITICAL);
		}

		m_DetourNavQuery = dtAllocNavMeshQuery();
		if (!m_DetourNavQuery)
		{
			Logger::Log("Could not create Detour navquery.", "NavManager", CRITICAL);
		}

		status = m_DetourNavQuery->init(m_DetourNavMesh, 2048);
		if (!status)
		{
			Logger::Log("Could not init Detour navmesh query", "NavManager", CRITICAL);
		}

		m_Meshes.clear();

		Ref<NavMesh> navMesh = CreateRef<NavMesh>(m_DetourNavMesh, m_DetourNavQuery);
		return navMesh;
	}

	void NavManager::DrawNavMesh()
	{
		if (!m_DetourNavMesh)
		{
			return;
		}

		//duDebugDrawNavMeshBVTree(&m_DebugDrawer, *m_DetourNavMesh);
		//duDebugDrawNavMeshNodes(&m_DebugDrawer, *m_DetourNavQuery);
		/*duDebugDrawNavMesh(&m_DebugDrawer, *m_DetourNavMesh, DU_DRAWNAVMESH_OFFMESHCONS);

		if (Engine::GetCurrentScene()->EntityExists("test"))
		{
			Ref<Scene> scene = Engine::GetCurrentScene();
			Entity testEntity = scene->GetEntity("test");
			auto& transform = testEntity.GetComponent<TransformComponent>();

			dtQueryFilter filter;
			Vector3 entityPosition = transform.GetGlobalPosition();
			float searchPosition[3] = { entityPosition.x, entityPosition.y, entityPosition.z };
			float searchExtent[3] = { 102.0f, 140.0f, 120.0f };
			dtStatus status;
			dtPolyRef resultPoly = 0;
			float nearestPoint[3];

			filter.setIncludeFlags(1);

			status = m_DetourNavQuery->findRandomPoint(&filter, frand, &resultPoly, nearestPoint);

			status = m_DetourNavQuery->findNearestPoly(searchPosition, searchExtent, &filter, &resultPoly, nearestPoint);

			if (resultPoly)
			{
				duDebugDrawNavMeshPoly(&m_DebugDrawer, *m_DetourNavMesh, resultPoly, duRGBA(0, 255, 0, 255));
			}

			if (scene->EntityExists("test2"))
			{
				static const int MAX_POLYS = 256;
				dtPolyRef m_polys[MAX_POLYS];
				Entity testEntity2 = scene->GetEntity("test2");
				auto& transform2 = testEntity2.GetComponent<TransformComponent>();

				Vector3 entityPosition2 = transform2.GetGlobalPosition();
				float searchPosition2[3] = { entityPosition2.x, entityPosition2.y, entityPosition2.z };
				float searchExtent2[3] = { 102.0f, 140.0f, 120.0f };
				dtStatus statu2s;
				dtPolyRef resultPoly2 = 0;
				float nearestPoint2[3];

				status = m_DetourNavQuery->findNearestPoly(searchPosition2, searchExtent2, &filter, &resultPoly2, nearestPoint2);
				if (resultPoly2)
				{
					duDebugDrawNavMeshPoly(&m_DebugDrawer, *m_DetourNavMesh, resultPoly2, duRGBA(0, 0, 255, 255));
				}

				int m_NumPolys;
				m_DetourNavQuery->findPath(resultPoly, resultPoly2, nearestPoint, nearestPoint2, &filter, m_polys, &m_NumPolys, MAX_POLYS);

				for (int i = 0; i < m_NumPolys; i++)
				{
					duDebugDrawNavMeshPoly(&m_DebugDrawer, *m_DetourNavMesh, m_polys[i], duRGBA(255, 255, 0, 255));
				}
			}
		}*/
	}
}
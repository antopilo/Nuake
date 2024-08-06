#include "NavMesh.h"
#include "Recast.h"
#include "src/Core/String.h"
#include <DetourNavMeshBuilder.h>
#include "DetourNavMeshQuery.h"
#include <src/Core/Logger.h>

namespace Nuake {

	struct NavMeshTileHeader
	{
		dtTileRef tileRef;
		int dataSize;
	};

	struct NavMeshSetHeader
	{
		int magic;
		int version;
		int numTiles;
		dtNavMeshParams params;
	};

	NavMesh::NavMesh(dtNavMesh* navMesh, dtNavMeshQuery* navMeshQuery)
	{
		m_DetourNavMesh = navMesh;
		m_DetourNavQuery = navMeshQuery;
	}

	int NavMesh::FindNearestPolygon(const Vector3& searchPosition, const Vector3& searchBox)
	{
		float nearestPoint[3];
		float searchPos[3] = { searchPosition.x, searchPosition.y, searchPosition.z };
		float searchExtent[3] = { searchBox.x, searchBox.y, searchBox.z};

		dtQueryFilter filter;
		filter.setIncludeFlags(1);

		dtPolyRef resultPoly = 0;
		dtStatus status = m_DetourNavQuery->findNearestPoly(searchPos, searchExtent, &filter, &resultPoly, nearestPoint);
		return resultPoly;
	}

	std::vector<Vector3> NavMesh::FindPath(int polyStart, int polyEnd, const Vector3& nearestPointStart, const Vector3& nearestPointEnd, const int maxPoly)
	{
		dtNavMesh* mesh = dtAllocNavMesh();

		return std::vector<Vector3>();
	}

	json NavMesh::Serialize()
	{
		BEGIN_SERIALIZE();

		// Get number of tiles
		nlohmann::json headerJson;
		headerJson["magic"] = 1337;
		headerJson["version"] = 1;

		int numTiles = 0;
		const auto* navMesh = m_DetourNavMesh;
		for (int i = 0; i < m_DetourNavMesh->getMaxTiles(); i++)
		{
			const dtMeshTile* tile = navMesh->getTile(i);
			if (!tile || !tile->header || !tile->dataSize) continue;
			numTiles++;
		}

		const dtNavMeshParams* params = navMesh->getParams();
		if (params) 
		{
			nlohmann::json paramsJson;
			paramsJson["orig[0]"] = params->orig[0];
			paramsJson["orig[1]"] = params->orig[1];
			paramsJson["orig[2]"] = params->orig[2];
			paramsJson["tileWidth"] = params->tileWidth;
			paramsJson["tileHeight"] = params->tileHeight;
			paramsJson["maxTiles"] = params->maxTiles;
			paramsJson["maxPolys"] = params->maxPolys;

			headerJson["params"] = paramsJson;
		}

		headerJson["numTiles"] = numTiles;

		std::vector<nlohmann::json> tilesJson;
		for (int i = 0; i < m_DetourNavMesh->getMaxTiles(); i++)
		{
			const dtMeshTile* tile = navMesh->getTile(i);
			if (!tile || !tile->header || !tile->dataSize) continue;
			
			nlohmann::json tileJson;
			tileJson["tileRef"] = m_DetourNavMesh->getTileRef(tile);
			tileJson["dataSize"] = tile->dataSize;

			// Serialize tile data as base64
			std::vector<uint8_t> tileData(tile->data, tile->data + tile->dataSize);
			tileJson["data"] = String::Base64Encode(tileData);

			tilesJson.push_back(tileJson);
		}

		j["header"] = headerJson;
		j["tiles"] = tilesJson;

		END_SERIALIZE();
	}

	bool NavMesh::Deserialize(const json& j)
	{
		if (j.contains("header"))
		{
			int magicNumber = j["header"]["magic"];
			int version = j["header"]["version"];
			int numTiles = j["header"]["numTiles"];
		}

		//dtStatus status = mesh->init(&header.params);
		dtNavMesh* mesh = dtAllocNavMesh();

		dtNavMeshParams params;
		params.orig[0] = j["header"]["params"]["orig[0]"].get<float>();
		params.orig[1] = j["header"]["params"]["orig[1]"].get<float>();
		params.orig[2] = j["header"]["params"]["orig[2]"].get<float>();
		params.tileWidth = j["header"]["params"]["tileWidth"].get<float>();
		params.tileHeight = j["header"]["params"]["tileHeight"].get<float>();
		params.maxTiles = j["header"]["params"]["maxTiles"].get<int>();
		params.maxPolys = j["header"]["params"]["maxPolys"].get<int>();

		dtStatus status = mesh->init(&params);
		if (dtStatusFailed(status)) 
		{
			return false;
		}

		for (const auto& tileJson : j["tiles"]) 
		{
			auto dataSize = tileJson["dataSize"].get<int>();
			std::string base64Data = tileJson["data"].get<std::string>();

			// Decode Base64 data
			std::vector<uint8_t> tileData = String::Base64Decode(base64Data);

			if (dataSize != tileData.size()) 
			{
				Logger::Log("Tile data size mismatch", "navmesh", CRITICAL);
				continue;
			}

			auto data = new uint8_t[dataSize];
			std::memcpy(data, tileData.data(), dataSize);

			mesh->addTile(data, dataSize, DT_TILE_FREE_DATA, tileJson["tileRef"].get<int>(), 0);
		}


		m_DetourNavMesh = mesh;

		m_DetourNavQuery = dtAllocNavMeshQuery();
		m_DetourNavQuery->init(m_DetourNavMesh, 2048);
		return true;
	}

	NavMesh::~NavMesh()
	{
		delete m_DetourNavMesh;
		delete m_DetourNavQuery;
	}
}
#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Rendering/Mesh/Mesh.h"
#include "Nuake/AI/NavMesh.h"

#include "NavMeshDebugDrawer.h"

class rcContext;
class dtNavMesh;
class dtNavMeshQuery;


namespace Nuake {

	class RecastConfig;

	struct MeshTransformKeyPair
	{
		Ref<Mesh> mesh;
		Matrix4 transform;
	};

	class NavManager
	{
	public:
		static NavManager& Get()
		{
			static NavManager instance;
			return instance;
		}

		void Initialize();
		void PushMesh(const Ref<Mesh>& mesh, const Matrix4& transform);

		Ref<NavMesh> BuildNavMesh(const RecastConfig& config);

		void DrawNavMesh();

		void Cleanup();

	private:
		NavManager() = default;
		~NavManager() = default;

		std::vector<MeshTransformKeyPair> m_Meshes;

		NavMeshDebugDrawer m_DebugDrawer;

		Ref<rcContext> m_RecastContext;
		dtNavMesh* m_DetourNavMesh;
		dtNavMeshQuery* m_DetourNavQuery;
	};
}
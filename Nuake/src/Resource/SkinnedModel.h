#pragma once
#include "src/Core/Core.h"
#include "src/Rendering/Mesh/SkinnedMesh.h"
#include "src/Resource/Resource.h"
#include "src/Resource/Serializable.h"
#include "src/Resource/SkeletonNode.h"


namespace Nuake
{
	class SkinnedModel : public Resource, ISerializable
	{
	private:
		std::vector<Ref<SkinnedMesh>> m_Meshes;

		SkeletonNode m_SkeletonRoot;

	public:
		SkinnedModel();
		SkinnedModel(const std::string path);
		~SkinnedModel();

		void SetSkeletonRootNode(SkeletonNode& root)
		{
			m_SkeletonRoot = std::move(root);
		}

		void AddMesh(Ref<SkinnedMesh> mesh);
		std::vector<Ref<SkinnedMesh>>& GetMeshes();

		json Serialize() override;
		bool Deserialize(const json& j) override;
	};
}
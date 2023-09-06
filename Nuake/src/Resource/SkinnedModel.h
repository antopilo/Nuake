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

		uint32_t m_CurrentAnimation = 0;
		uint32_t m_NumAnimation = 0;
		std::vector<Ref<SkeletalAnimation>> m_Animations;

	public:
		SkinnedModel();
		SkinnedModel(const std::string path);
		~SkinnedModel();

		SkeletonNode& GetSkeletonRootNode() { return m_SkeletonRoot; }
		void SetSkeletonRootNode(SkeletonNode& root)
		{
			m_SkeletonRoot = std::move(root);
		}

		void AddMesh(Ref<SkinnedMesh> mesh);
		std::vector<Ref<SkinnedMesh>>& GetMeshes();

		void SetAnimations(const std::vector<Ref<SkeletalAnimation>> animations);
		void AddAnimation(Ref<SkeletalAnimation> animation);
		Ref<SkeletalAnimation> GetCurrentAnimation();
		void PlayAnimation(uint32_t animationId);

		json Serialize() override;
		bool Deserialize(const json& j) override;
	};
}
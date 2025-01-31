#pragma once
#include "Nuake/Core/Core.h"
#include "Nuake/Rendering/Mesh/SkinnedMesh.h"
#include "Nuake/Resource/Resource.h"
#include "Nuake/Resource/Serializable.h"
#include "Nuake/Resource/SkeletonNode.h"


namespace Nuake
{
	class SkinnedModel : public Resource, ISerializable
	{
	private:
		std::vector<Ref<SkinnedMesh>> m_Meshes;

		SkeletonNode m_SkeletonRoot;

		uint32_t m_CurrentAnimation = 0;
		uint32_t m_PreviousAnimation = 0;
		float m_TransitionBlendTime = 0.0f;
		uint32_t m_NumAnimation = 0;
		std::vector<Ref<SkeletalAnimation>> m_Animations;

	public:
		bool IsPlaying = true;

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

		std::vector<Ref<SkeletalAnimation>> GetAnimations() const { return m_Animations; }
		void SetAnimations(const std::vector<Ref<SkeletalAnimation>> animations);
		void AddAnimation(Ref<SkeletalAnimation> animation);

		float GetCurrentBlendTime();
		void SetCurrentBlendTime(float time);
		Ref<SkeletalAnimation> GetCurrentAnimation();
		Ref<SkeletalAnimation> GetPreviousAnimation();
		void PlayAnimation(uint32_t animationId);
		uint32_t GetCurrentAnimationIndex() const { return m_CurrentAnimation; }
		uint32_t GetAnimationsCount() const { return m_NumAnimation; }

		json Serialize() override;
		bool Deserialize(const json& j) override;
	};
}
#include "Nuake/Resource/ModelLoader.h"

#include "Nuake/Resource/SkinnedModel.h"
#include "Nuake/FileSystem/FileSystem.h"
#include "Nuake/Core/Logger.h"


namespace Nuake
{
	SkinnedModel::SkinnedModel(const std::string path) : m_Meshes(std::vector<Ref<SkinnedMesh>>())
	{
		this->Path = path;
	}

	SkinnedModel::SkinnedModel() : m_Meshes(std::vector<Ref<SkinnedMesh>>())
	{}

	SkinnedModel::~SkinnedModel() {}

	void SkinnedModel::AddMesh(Ref<SkinnedMesh> mesh)
	{
		m_Meshes.push_back(mesh);
	}

	std::vector<Ref<SkinnedMesh>>& SkinnedModel::GetMeshes()
	{
		return m_Meshes;
	}

	void SkinnedModel::SetAnimations(const std::vector<Ref<SkeletalAnimation>> animations)
	{
		m_NumAnimation = static_cast<uint32_t>(animations.size());
		m_CurrentAnimation = 0;
		m_Animations = animations;
	}

	void SkinnedModel::AddAnimation(Ref<SkeletalAnimation> animation)
	{
		m_NumAnimation++;
		m_Animations.push_back(std::move(animation));
	}

	void SkinnedModel::SetCurrentBlendTime(float blendTime)
	{
		m_TransitionBlendTime = std::max(blendTime, 0.0f);
	}

	float SkinnedModel::GetCurrentBlendTime()
	{
		return m_TransitionBlendTime;
	}

	Ref<Nuake::SkeletalAnimation> SkinnedModel::GetCurrentAnimation()
	{
		if (m_CurrentAnimation < m_NumAnimation)
		{
			return m_Animations[m_CurrentAnimation];
		}

		Logger::Log("Cannot get animation if no animation exists", "skinned model", WARNING);
		return nullptr;
	}

	Ref<Nuake::SkeletalAnimation> SkinnedModel::GetPreviousAnimation()
	{
		if (m_PreviousAnimation < m_NumAnimation)
		{
			return m_Animations[m_PreviousAnimation];
		}

		Logger::Log("Cannot get animation if no animation exists", "skinned model", WARNING);
		return nullptr;
	}

	void SkinnedModel::PlayAnimation(uint32_t animationId)
	{
		if (animationId >= m_NumAnimation)
		{
			Logger::Log("Cannot play animation, index out of range", "skinned model", CRITICAL);
			return;
		}

		GetCurrentAnimation()->SetCurrentTime(0.0f); // Reset previous animation

		m_PreviousAnimation = m_CurrentAnimation;
		m_CurrentAnimation = animationId;

		m_TransitionBlendTime = 0.6f;
	}

	json SkinnedModel::Serialize()
	{
		BEGIN_SERIALIZE();

		if (this->Path != "")
		{
			j["Path"] = this->Path;
			j["SkeletonNode"] = m_SkeletonRoot.Serialize();
		}
		else
		{
			for (uint32_t i = 0; i < std::size(m_Meshes); i++)
			{
				j["Meshes"][i] = m_Meshes[i]->Serialize();
			}

			j["m_CurrentAnimation"] = m_CurrentAnimation;
			j["m_NumAnimation"] = m_NumAnimation;

			uint32_t a = 0;
			for (auto& animation : m_Animations)
			{
				j["m_Animations"][a] = animation->Serialize();
				a++;
			}
		}
		
		END_SERIALIZE();
	}

	bool SkinnedModel::Deserialize(const json& j)
	{
		if (j.contains("Path"))
		{

			ModelLoader loader;
			auto otherModel = loader.LoadSkinnedModel(j["Path"], false);
			m_Meshes = otherModel->GetMeshes();
			m_Animations = otherModel->GetAnimations();
			m_SkeletonRoot = otherModel->GetSkeletonRootNode();
			m_NumAnimation = static_cast<uint32_t>(m_Animations.size());
			m_CurrentAnimation = 0;

			if (j.contains("SkeletonNode"))
			{
				SkeletonNode skeletonNode;
				skeletonNode.Deserialize(j["SkeletonNode"]);
				m_SkeletonRoot = std::move(skeletonNode);
			}

			this->Path = j["Path"];
		}
		else
		{
			if (!j.contains("Meshes"))
			{
				return true;
			}

			for (auto& m : j["Meshes"])
			{
				auto mesh = CreateRef<SkinnedMesh>();
				mesh->Deserialize(m);

				m_Meshes.push_back(mesh);
			}
		}

		return true;
	}
}
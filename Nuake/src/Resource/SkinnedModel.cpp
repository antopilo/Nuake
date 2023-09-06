#include "src/Resource/ModelLoader.h"

#include "src/Resource/SkinnedModel.h"
#include "src/Core/FileSystem.h"
#include "src/Core/Logger.h"


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

	Ref<Nuake::SkeletalAnimation> SkinnedModel::GetCurrentAnimation()
	{
		if (m_CurrentAnimation < m_NumAnimation)
		{
			return m_Animations[m_CurrentAnimation];
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

		m_CurrentAnimation = animationId;
	}

	json SkinnedModel::Serialize()
	{
		BEGIN_SERIALIZE();

		if (this->Path != "")
		{
			j["Path"] = this->Path;
		}
		else
		{
			for (uint32_t i = 0; i < std::size(m_Meshes); i++)
			{
				j["Meshes"][i] = m_Meshes[i]->Serialize();
			}
		}
		END_SERIALIZE();
	}

	bool SkinnedModel::Deserialize(const json& j)
	{
		if (j.contains("Path"))
		{
			this->IsEmbedded = true;

			ModelLoader loader;
			auto otherModel = loader.LoadSkinnedModel(j["Path"], false);
			m_Meshes = otherModel->GetMeshes();

			this->Path = j["Path"];
		}
		else
		{
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
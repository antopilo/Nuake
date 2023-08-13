#pragma once
#include "src/Core/Core.h"
#include "src/Rendering/Mesh/SkinnedMesh.h"
#include "src/Resource/Resource.h"
#include "src/Resource/Serializable.h"


namespace Nuake
{
	class SkinnedModel : public Resource, ISerializable
	{
	private:
		std::vector<Ref<SkinnedMesh>> m_Meshes;

	public:
		SkinnedModel();
		SkinnedModel(const std::string path);
		~SkinnedModel();

		void AddMesh(Ref<SkinnedMesh> mesh);
		std::vector<Ref<SkinnedMesh>>& GetMeshes();

		json Serialize() override;
		bool Deserialize(const json& j) override;
	};
}
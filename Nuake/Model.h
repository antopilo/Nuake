#pragma once
#include "src/Core/Core.h"

#include "src/Resource/Resource.h"
#include "src/Resource/Serializable.h"

namespace Nuake
{
	class Mesh;

	class Model : Resource, ISerializable
	{
	private:
		std::vector<Ref<Mesh>> m_Meshes;

	public:
		Model();
		~Model();

		bool LoadModel(const std::string& path);

		std::vector<Ref<Mesh>>& GetMeshes();

		json Serialize() override;
		bool Deserialize(const std::string& data) override;
	};
}
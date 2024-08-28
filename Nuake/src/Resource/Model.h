#pragma once
#include "src/Core/Core.h"

#include "src/Resource/Resource.h"
#include "src/Resource/Serializable.h"
#include "src/Rendering/Mesh/Mesh.h"

namespace Nuake
{
	class Model : public Resource, ISerializable
	{
	private:
		std::vector<Ref<Mesh>> m_Meshes;
	public:
		Model();
		Model(const std::string path);
		~Model();

		void AddMesh(Ref<Mesh> mesh);
		std::vector<Ref<Mesh>>& GetMeshes();

		json SerializeData();
		json Serialize() override;
		bool Deserialize(const json& j) override;
	};
}
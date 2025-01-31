#pragma once
#include "Nuake/Core/Core.h"

#include "Nuake/Resource/Resource.h"
#include "Nuake/Resource/Serializable.h"
#include "Nuake/Rendering/Mesh/Mesh.h"


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
#pragma once
#include "src/Core/Core.h"
#include "src/Resource/Serializable.h"

namespace Nuake
{
	class Mesh;
	class Model;
	class Material;

	class BinarySerializer : public ISerializer
	{
	public: 
		BinarySerializer() = default;
		~BinarySerializer() = default;

	public:
		UUID DeserializeUUID(const std::string& path) override;

		Ref<Material> DeserializeMaterial(const std::string& path) override;
		bool SerializeMaterial(const std::string& path, Ref<Material> material) override;

		Ref<Model> DeserializeModel(const std::string& path) override;
		bool SerializeModel(const std::string& path, Ref<Model> model) override;

        Ref<Mesh> DeserializeMesh(const std::string& path) override;
		bool SerializeMesh(const std::string& path, Ref<Mesh> mesh) override;
	};
}
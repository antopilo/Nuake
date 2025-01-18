#include "MeshResolver.h"

#include "src/Resource/Model.h"
#include "src/Rendering/Textures/Material.h"

#include "src/Resource/ResourceManager.h"
#include "src/Resource/Serializer/BinarySerializer.h"

using namespace Nuake;

Ref<Resource> MeshResolver::Resolve(const Ref<File>& file)
{
	BinarySerializer deserializer = BinarySerializer();
	return deserializer.DeserializeModel(file->GetRelativePath());
}

Ref<Resource> MaterialResolver::Resolve(const Ref<File>& file)
{
	// Read json content
	std::string fileContent = FileSystem::ReadFile(file->GetRelativePath());
	json jsonData = json::parse(fileContent);

	// Create material from json data
	Ref<Material> material = CreateRef<Material>();
	material->Deserialize(jsonData);

	return material;
}

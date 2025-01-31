#include "MeshResolver.h"

#include "Nuake/Resource/Model.h"
#include "Nuake/Rendering/Textures/Material.h"

#include "Nuake/Resource/ResourceManager.h"
#include "Nuake/Resource/Serializer/BinarySerializer.h"

using namespace Nuake;

UUID MeshResolver::ResolveUUID(const Ref<File>& file)
{
	BinarySerializer deserializer = BinarySerializer();
	return deserializer.DeserializeUUID(file->GetRelativePath());
}

Ref<Resource> MeshResolver::Resolve(const Ref<File>& file)
{
	BinarySerializer deserializer = BinarySerializer();
	return deserializer.DeserializeModel(file->GetRelativePath());
}

UUID MaterialResolver::ResolveUUID(const Ref<File>& file)
{
	std::string fileContent = FileSystem::ReadFile(file->GetRelativePath());
	json jsonData = json::parse(fileContent);
	if (jsonData.contains("UUID"))
	{
		return static_cast<uint64_t>(jsonData["UUID"]);
	}

	return UUID();
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

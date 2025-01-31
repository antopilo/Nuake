#include "SkyComponent.h"
#include "Nuake/FileSystem/File.h"
#include "Nuake/FileSystem/FileSystem.h"

using namespace Nuake;


json SkyComponent::Serialize()
{
	BEGIN_SERIALIZE();
	SERIALIZE_RES_FILE(SkyResourceFilePath);
	END_SERIALIZE();
}

bool SkyComponent::Deserialize(const json& j)
{
	DESERIALIZE_RES_FILE(SkyResourceFilePath);
	return true;
}
#include "EnvironmentComponent.h"
#include "src/FileSystem/File.h"
#include "src/FileSystem/FileSystem.h"

namespace Nuake
{
	json EnvironmentComponent::Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_RES_FILE(EnvResourceFilePath);
		END_SERIALIZE();
	}

	bool EnvironmentComponent::Deserialize(const json& j)
	{
		DESERIALIZE_RES_FILE(EnvResourceFilePath);
		return true;
	}
}
#include "SkyComponent.h"

using namespace Nuake;


json SkyComponent::Serialize()
{
	BEGIN_SERIALIZE();

	
	END_SERIALIZE();
}

bool SkyComponent::Deserialize(const json& j)
{
	
	return true;
}
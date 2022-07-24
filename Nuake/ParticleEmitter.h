#pragma once
#include "src/Resource/Serializable.h"

namespace Nuake
{
	class ParticleEmitter
	{
	public:


		json Serialize();
		bool Deserialize(const std::string& str);
	};
}
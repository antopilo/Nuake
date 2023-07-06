#pragma once
#include <string>
#include <vector>

#include "src/Rendering/Mesh/Mesh.h"
#include "src/Resource/Serializable.h"
#include "src/Scene/Systems/QuakeMapBuilder.h"

namespace Nuake {
	class QuakeMapComponent
	{
	public:
		std::vector<Ref<Mesh>> m_Meshes;
		std::string Path;
		float ScaleFactor = 1.0f;
		bool HasCollisions = false;
		bool AutoRebuild = false;

		json Serialize()
		{
			BEGIN_SERIALIZE();
			SERIALIZE_VAL(HasCollisions);
			SERIALIZE_VAL(Path);
			SERIALIZE_VAL(AutoRebuild);
			for (unsigned int i = 0; i < m_Meshes.size(); i++)
			{
				j["Meshes"][i] = m_Meshes[i]->Serialize();
			}
			
			END_SERIALIZE();
		}

		bool Deserialize(std::string str)
		{
			BEGIN_DESERIALIZE();
			if (j.contains("AutoRebuild"))
			{
				this->AutoRebuild = j["AutoRebuild"];
			}

			this->Path = j["Path"];
			this->HasCollisions = j["HasCollisions"];
			return true;
		}
	};
}

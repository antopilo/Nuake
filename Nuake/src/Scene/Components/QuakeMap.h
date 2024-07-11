#pragma once
#include <string>
#include <vector>

#include "src/Rendering/Mesh/Mesh.h"
#include "src/Resource/Serializable.h"
#include "src/Scene/Systems/QuakeMapBuilder.h"
#include "src/Scene/Entities/Entity.h"
#include "Engine.h"

namespace Nuake {

	class QuakeMapComponent
	{
	public:
		std::vector<Ref<Mesh>> m_Meshes;
		std::vector<Entity> m_Brushes;
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
			for (uint32_t i = 0; i < std::size(m_Brushes); i++)
			{
				j["Brushes"][i] = m_Brushes[i].GetID();
			}

			for (unsigned int i = 0; i < m_Meshes.size(); i++)
			{
				j["Meshes"][i] = m_Meshes[i]->Serialize();
			}
			
			END_SERIALIZE();
		}

		bool Deserialize(const json& j)
		{
			if (j.contains("AutoRebuild"))
			{
				this->AutoRebuild = j["AutoRebuild"];
			}

			if (j.contains("Brushes"))
			{
				for (auto& b : j["Brushes"])
				{
					//m_Brushes.push_back(Engine::GetCurrentScene()->GetEntityByID(b));
				}
			}

			this->Path = j["Path"];
			this->HasCollisions = j["HasCollisions"];
			return true;
		}
	};
}

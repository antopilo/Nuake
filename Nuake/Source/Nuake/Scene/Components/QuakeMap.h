#pragma once

#include "Component.h"
#include "FieldTypes.h"

#include "Nuake/FileSystem/File.h"
#include "Nuake/Rendering/Mesh/Mesh.h"
#include "Nuake/Resource/Serializable.h"
#include "Nuake/Scene/Systems/QuakeMapBuilder.h"
#include "Nuake/Scene/Entities/Entity.h"
#include "Engine.h"

#include <string>
#include <vector>
#include "Nuake/Resource/Serializer/BinarySerializer.h"


namespace Nuake {

	class QuakeMapComponent : public Component
	{
		NUAKECOMPONENT(QuakeMapComponent, "Quake Map")

		static void InitializeComponentClass()
		{
			BindComponentField<&QuakeMapComponent::HasCollisions>("HasCollisions", "Has Collisions");
			BindComponentField<&QuakeMapComponent::Path>("Path", "Path");
			BindComponentField<&QuakeMapComponent::AutoRebuild>("AutoRebuild", "Auto Rebuild");

			BindAction<&QuakeMapComponent::ActionRebuild>("Rebuild", "Rebuild");
		}

	public:
		bool HasCollisions = false;
		ResourceFile Path;
		bool AutoRebuild = false;
		float ScaleFactor = 1.f;
		
		bool rebuildNextTick = false;
		
		std::vector<Ref<Mesh>> m_Meshes;
		std::vector<Entity> m_Brushes;
		std::vector<int> m_SerializedBrushIDs;

		void ActionRebuild();

		json Serialize()
		{
			BEGIN_SERIALIZE();
			SERIALIZE_VAL(HasCollisions);
			SERIALIZE_RES_FILE(Path);
			SERIALIZE_VAL(AutoRebuild);

			for (uint32_t i = 0; i < std::size(m_Brushes); i++)
			{
				j["Brushes"][i] = m_Brushes[i].GetID();
			}

			BinarySerializer serializer;
			for (unsigned int i = 0; i < m_Meshes.size(); i++)
			{
				const std::string& path = FileSystem::RelativeToAbsolute("mesh" + std::to_string(i) + ".mesh");
				serializer.SerializeMesh(path, m_Meshes[i]);

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

			m_Brushes.clear();

			if (j.contains("Brushes"))
			{
				for (auto& b : j["Brushes"])
				{
					m_SerializedBrushIDs.push_back(b);
				}
			}

			DESERIALIZE_RES_FILE(Path);
			this->HasCollisions = j["HasCollisions"];
			return true;
		}

		void PostDeserialize(Scene& scene)
		{
			m_Brushes.clear();

			for (auto& b : m_SerializedBrushIDs)
			{
				m_Brushes.push_back(scene.GetEntityByID(b));
			}

			m_SerializedBrushIDs.clear();
		}
	};

	inline void QuakeMapComponent::ActionRebuild()
	{
		rebuildNextTick = true;
	}
}

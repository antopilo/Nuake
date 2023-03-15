#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"

#include "src/Rendering/Textures/Material.h"
#include "src/Rendering/Mesh/Mesh.h"
#include "src/Core/Physics/Rigibody.h"

namespace Nuake {
	class BSPBrushComponent
	{
	public:
		std::vector<Ref<Mesh>> Meshes;
		std::vector<std::vector<Vector3>> Hulls;

		std::vector<Ref<Material>> Materials;
		std::vector<Ref<Physics::RigidBody>> Rigidbody;

		std::string target = "";
		std::vector<Entity> Targets;

		bool IsSolid = true;
		bool IsTrigger = false;
		bool IsTransparent = false;
		bool IsFunc = false;

		BSPBrushComponent() 
		{
			Meshes = std::vector<Ref<Mesh>>();
			Materials = std::vector<Ref<Material>>();
			Rigidbody = std::vector<Ref<Physics::RigidBody>>();
		}

		json Serialize()
		{
			BEGIN_SERIALIZE();

			for (uint32_t i = 0; i < Meshes.size(); i++)
			{
				j["Meshes"][i] = Meshes[i]->Serialize();
			}

			j["IsSolid"] = IsSolid;
			END_SERIALIZE();
		}

		bool Deserialize(const std::string& str)
		{
			BEGIN_DESERIALIZE();

			if (j.contains("IsSolider"))
			{
				IsSolid = j["IsSolid"];
			}

			return true;
		}
	};
}

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
		std::vector< Ref<Material>> Materials;
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

			for (unsigned int i = 0; i < Meshes.size(); i++)
			{
				j["Meshes"][i] = Meshes[i]->Serialize();
			}

			END_SERIALIZE();
		}

		bool Deserialize()
		{

		}
	};
}

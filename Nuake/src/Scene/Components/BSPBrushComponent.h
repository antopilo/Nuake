#pragma once
#include "src/Core/Core.h"
#include "src/Core/Maths.h"

#include "src/Rendering/Textures/Material.h"
#include "src/Rendering/Mesh/Mesh.h"
#include "src/Physics/Rigibody.h"

namespace Nuake {
	class BSPBrushComponent
	{
	public:
		//std::vector<Ref<Mesh>> Meshes;
		std::vector<std::vector<Vector3>> Hulls;

		std::vector<Ref<Material>> Materials;
		std::vector<Ref<Physics::RigidBody>> Rigidbody;

		std::string target = "";
		std::string TargetName = "";
		std::vector<Entity> Targets;

		bool IsSolid = true;
		bool IsTrigger = false;
		bool IsTransparent = false;
		bool IsFunc = false;

		BSPBrushComponent() 
		{
			//Meshes = std::vector<Ref<Mesh>>();
			Materials = std::vector<Ref<Material>>();
			Rigidbody = std::vector<Ref<Physics::RigidBody>>();
			Hulls = std::vector<std::vector<Vector3>>();
		}

		json Serialize()
		{
			BEGIN_SERIALIZE();

			for (uint32_t i = 0; i < Hulls.size(); i++)
			{
				json hullPointsJson;

				size_t hullSize = Hulls[i].size();
				for (uint32_t j = 0; j < hullSize; j++)
				{
					hullPointsJson[j]["x"] = Hulls[i][j].x;
					hullPointsJson[j]["y"] = Hulls[i][j].y;
					hullPointsJson[j]["z"] = Hulls[i][j].z;
				}
				j["Hulls"][i] = hullPointsJson;
			}

			SERIALIZE_VAL(target);
			SERIALIZE_VAL(TargetName);
			j["IsSolid"] = IsSolid;
			SERIALIZE_VAL(IsTrigger);
			END_SERIALIZE();
		}

		bool Deserialize(const json& j)
		{
			if (j.contains("IsSolid"))
			{
				IsSolid = j["IsSolid"];
			}

			if (j.contains("Hulls"))
			{
				for (auto& h : j["Hulls"])
				{
					std::vector<Vector3> hull;
					hull.reserve(h.size());
					for (auto& point : h)
					{
						Vector3 pointPos;
						DESERIALIZE_VEC3(point, pointPos);
						hull.push_back((pointPos));
					}

					Hulls.push_back(hull);
				}
			}

			DESERIALIZE_VAL(target);
			DESERIALIZE_VAL(TargetName);
			DESERIALIZE_VAL(IsTrigger);
			return true;
		}
	};
}

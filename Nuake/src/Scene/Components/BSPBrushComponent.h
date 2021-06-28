#pragma once
#include "../Core/Core.h"
#include "../Core/Maths.h"

#include "../Rendering/Textures/Material.h"
#include "../Rendering/Mesh/Mesh.h"
#include <src/Core/Physics/Rigibody.h>

class BSPBrushComponent {
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

	BSPBrushComponent() {
		Meshes = std::vector<Ref<Mesh>>();
		Materials = std::vector<Ref<Material>>();
		Rigidbody = std::vector<Ref<Physics::RigidBody>>();
	}
};
#pragma once
#include "../Core/Core.h"
#include "../Core/Maths.h"

#include "../Rendering/Textures/Material.h"
#include "../Rendering/Mesh/Mesh.h"

class BSPBrushComponent {
public:
	std::vector<Ref<Mesh>> Meshes;
	std::vector< Ref<Material>> Materials;

	bool IsSolid = true;
	bool IsTrigger = false;
	bool IsTransparent = false;

	BSPBrushComponent() {
		Meshes = std::vector<Ref<Mesh>>();
		Materials = std::vector<Ref<Material>>();
	}
};
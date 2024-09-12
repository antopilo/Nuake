#pragma once

#include "Component.h"

#include "src/Core/Core.h"
#include "src/Resource/Serializable.h"
#include "src/Rendering/Textures/Texture.h"
#include "src/Rendering/Mesh/Mesh.h"

namespace Nuake 
{
	class SpriteComponent : public Component
	{
		NUAKECOMPONENT(SpriteComponent, "Sprite")

	public:
		bool Billboard;
		bool LockYRotation;
		bool PositionFacing;

		std::string SpritePath;
		Ref<Mesh> SpriteMesh;

		SpriteComponent();

	public:
		bool LoadSprite();

		json Serialize();
		bool Deserialize(const json& j);
	};
}

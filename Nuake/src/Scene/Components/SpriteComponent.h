#pragma once

#include "Component.h"
#include "FieldTypes.h"

#include "src/Core/Core.h"
#include "src/Resource/Serializable.h"
#include "src/Rendering/Textures/Texture.h"
#include "src/Rendering/Mesh/Mesh.h"

namespace Nuake 
{
	class SpriteComponent : public Component
	{
		NUAKECOMPONENT(SpriteComponent, "Sprite")

		static void InitializeComponentClass()
		{
			BindComponentField<&SpriteComponent::Billboard>("Billboard", "Billboard");
			BindComponentField<&SpriteComponent::LockYRotation>("LockYRotation", "Lock Y Rotation");
			BindComponentField<&SpriteComponent::PositionFacing>("PositionFacing", "Position Facing");
			BindComponentField<&SpriteComponent::SpritePath>("SpritePath", "Sprite Path");
		}

	public:
		bool Billboard;
		bool LockYRotation;
		bool PositionFacing;

		ResourceFile SpritePath;
		Ref<Mesh> SpriteMesh;

		SpriteComponent();

	public:
		bool LoadSprite();

		json Serialize();
		bool Deserialize(const json& j);
	};
}

#pragma once

#include "Component.h"
#include "FieldTypes.h"

#include "Nuake/Core/Core.h"
#include "Nuake/Resource/Serializable.h"
#include "Nuake/Rendering/Textures/Texture.h"
#include "Nuake/Rendering/Mesh/Mesh.h"

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
			//BindComponentField<&SpriteComponent::SpritePath>("SpritePath", "Sprite Path");
			BindComponentProperty<&SpriteComponent::SetSprite, &SpriteComponent::GetSprite>("SpritePath", "Sprite Path");
				ResourceFileRestriction("_Image");
		}

	public:
		bool Billboard;
		bool LockYRotation;
		bool PositionFacing;

		ResourceFile SpritePath;
		Ref<Mesh> SpriteMesh;

		SpriteComponent();

	public:
		void SetSprite(ResourceFile file);
		ResourceFile GetSprite()
		{ 
			return SpritePath;
		}

		bool LoadSprite();

		json Serialize();
		bool Deserialize(const json& j);
	};
}

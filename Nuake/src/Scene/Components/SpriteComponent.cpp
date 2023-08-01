#include "SpriteComponent.h"

#include "src/Rendering/Textures/TextureManager.h"

namespace Nuake
{
	SpriteComponent::SpriteComponent() :
		Billboard(false),
		LockYRotation(false),
		SpritePath("")
	{

	}

	bool SpriteComponent::LoadSprite()
	{
		const auto texture = TextureManager::Get()->GetTexture(SpritePath);
		Sprite = texture;
		return true;
	}

	json SpriteComponent::Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_VAL(Billboard)
		SERIALIZE_VAL(LockYRotation)
		SERIALIZE_VAL(SpritePath)
		END_SERIALIZE();
	}

	bool SpriteComponent::Deserialize(const std::string& str)
	{
		BEGIN_DESERIALIZE();

		if (j.contains("Billboard"))
		{
			Billboard = j["Billboard"];
		}

		if (j.contains("LockYRotation"))
		{
			LockYRotation = j["LockYRotation"];
		}

		if (j.contains("SpritePath"))
		{
			SpritePath = j["SpritePath"];
			LoadSprite();
		}

		return true;
	}
}
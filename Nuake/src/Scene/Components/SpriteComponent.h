#pragma once
#include "src/Core/Core.h"
#include "src/Resource/Serializable.h"
#include "src/Rendering/Textures/Texture.h"

namespace Nuake 
{
	class SpriteComponent
	{
	public:
		bool Billboard;
		bool LockYRotation;
		std::string SpritePath;
		Ref<Texture> Sprite;

		SpriteComponent();

	public:
		bool LoadSprite();

		json Serialize();
		bool Deserialize(const std::string& str);
	};
}

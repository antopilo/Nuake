#include "SpriteComponent.h"

#include "src/Core/FileSystem.h"
#include "src/Rendering/Textures/TextureManager.h"
#include "src/Rendering/Textures/MaterialManager.h"
#include "src/Rendering/Vertex.h"

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
		std::vector<Vertex> quadVertices =
		{
			{ Vector3(-1.0f, -1.0f, 0.0f), Vector2(0, 0), Vector3(0, 0, -1) },
			{ Vector3(1.0f,  1.0f, 0.0f), Vector2(1.0f, 1.0f), Vector3(0, 0, -1) },
			{ Vector3(-1.0f,  1.0f, 0.0f), Vector2(0.0f, 1.0f), Vector3(0, 0, -1) },
			{ Vector3(1.0f,  -1.0f, 0.0f), Vector2(1.0f, 0.0f), Vector3(0, 0, -1) },
			{ Vector3(-1.0f, -1.0f, 0.0f), Vector2(0.0f, 0.0f), Vector3(0, 0, -1) },
			{ Vector3(1.0f,   1.0f, 0.0f), Vector2(1.0f, 1.0f), Vector3(0, 0, -1) }
		};

		SpriteMesh = CreateRef<Mesh>();
		SpriteMesh->AddSurface(quadVertices, { 0, 1, 2, 3, 4, 5 });

		auto& material = MaterialManager::Get()->GetMaterial(FileSystem::Root + SpritePath);
		SpriteMesh->SetMaterial(material);

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

	bool SpriteComponent::Deserialize(const json& j)
	{
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
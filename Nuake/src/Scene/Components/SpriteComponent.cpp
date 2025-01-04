#include "SpriteComponent.h"

#include "src/FileSystem/File.h"
#include "src/FileSystem/FileSystem.h"
#include "src/Rendering/Textures/TextureManager.h"
#include "src/Rendering/Textures/MaterialManager.h"
#include "src/Rendering/Textures/Material.h"
#include "src/Rendering/Vertex.h"

namespace Nuake
{
	SpriteComponent::SpriteComponent() :
		Billboard(false),
		LockYRotation(false)
	{
	}

	void SpriteComponent::SetSprite(ResourceFile path)
	{
		if (!FileSystem::FileExists(path.GetRelativePath()))
		{
			return;
		}

		SpritePath = FileSystem::GetFile(path.GetRelativePath());

		std::vector<Vertex> quadVertices =
		{
			{ Vector3(-1.0f,  1.0f, 0.0f),	0.0f, Vector3(0, 0, 1), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
			{ Vector3(1.0f,  1.0f, 0.0f),	1.0f, Vector3(0, 0, 1), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
			{ Vector3(-1.0f, -1.0f, 0.0f),	0.0f, Vector3(0, 0, 1), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },

			{ Vector3(1.0f,  -1.0f, 0.0f),	1.0f, Vector3(0, 0, 1), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
			{ Vector3(-1.0f, -1.0f, 0.0f),	0.0f, Vector3(0, 0, 1), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
			{ Vector3(1.0f,   1.0f, 0.0f),	1.0f, Vector3(0, 0, 1), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) }
		};

		SpriteMesh = CreateRef<Mesh>();
		SpriteMesh->AddSurface(quadVertices, { 0, 1, 2, 3, 4, 5 });

		std::string absPath = "";
		if (SpritePath.file != nullptr && SpritePath.file->Exist())
		{
			absPath = SpritePath.file->GetAbsolutePath();
		}

		Ref<Material> material = MaterialManager::Get()->GetMaterial(absPath);
		bool hasNormal = material->HasNormal();
		SpriteMesh->SetMaterial(material);
	}

	bool SpriteComponent::LoadSprite()
	{

		return true;
	}

	json SpriteComponent::Serialize()
	{
		BEGIN_SERIALIZE();
		SERIALIZE_VAL(Billboard);
		SERIALIZE_VAL(LockYRotation);
		SERIALIZE_RES_FILE(SpritePath);
		SERIALIZE_VAL(PositionFacing);
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

		DESERIALIZE_RES_FILE(SpritePath);
		SetSprite(SpritePath);

		if (j.contains("PositionFacing"))
		{
			PositionFacing = j["PositionFacing"];
		}

		LoadSprite();

		return true;
	}
}
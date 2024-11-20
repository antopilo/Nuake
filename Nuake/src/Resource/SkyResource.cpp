#include "SkyResource.h"

using namespace Nuake;

SkyResource::SkyResource(const std::string &path) :
	cubemapTexture(nullptr)
{
	this->Path = path;
}

void SkyResource::SetTextureFace(SkyFaces face, const std::string& path)
{
	switch (face)
	{
	case SkyFaces::Top:
		UpTexturePath = path;
		break;
	case SkyFaces::Bottom:
		DownTexturePath = path;
		break;
	case SkyFaces::Left:
		LeftTexturePath = path;
		break;
	case SkyFaces::Right:
		RightTexturePath = path;
		break;
	case SkyFaces::Front:
		FrontTexturePath = path;
		break;
	case SkyFaces::Back:
		BackTexturePath = path;
		break;
	}

	ResetCubemap();
}

std::string SkyResource::GetFaceTexture(SkyFaces face) const
{
	switch (face)
	{
	case SkyFaces::Top:
		return UpTexturePath;
	case SkyFaces::Bottom:
		return DownTexturePath;
	case SkyFaces::Left:
		return LeftTexturePath;
	case SkyFaces::Right:
		return RightTexturePath;
	case SkyFaces::Front:
		return FrontTexturePath;
	case SkyFaces::Back:
		return BackTexturePath;
	}

	return "";
}

Ref<CubemapTexture> SkyResource::GetCubemap()
{
	if (cubemapTexture != nullptr)
	{
		return cubemapTexture;
	}

	cubemapTexture = CreateRef<CubemapTexture>(*this);

	return cubemapTexture;
}

void SkyResource::ResetCubemap()
{
	cubemapTexture = nullptr;
}

json SkyResource::Serialize()
{
	BEGIN_SERIALIZE();
	SERIALIZE_VAL(FrontTexturePath);
	SERIALIZE_VAL(BackTexturePath);
	SERIALIZE_VAL(UpTexturePath);
	SERIALIZE_VAL(DownTexturePath);
	SERIALIZE_VAL(LeftTexturePath);
	SERIALIZE_VAL(RightTexturePath);
	END_SERIALIZE();
}

bool SkyResource::Deserialize(const json& j)
{
	DESERIALIZE_VAL(FrontTexturePath);
	DESERIALIZE_VAL(BackTexturePath);
	DESERIALIZE_VAL(UpTexturePath);
	DESERIALIZE_VAL(DownTexturePath);
	DESERIALIZE_VAL(LeftTexturePath);
	DESERIALIZE_VAL(RightTexturePath);
	return true;
}

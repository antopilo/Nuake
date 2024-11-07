#pragma once

#include "src/Core/Core.h"
#include "src/Resource/Resource.h"
#include "src/Resource/Serializable.h"
#include "src/Rendering/Textures/Cubemap.h"

namespace Nuake
{
	enum class SkyFaces
	{
		Top, 
		Bottom,
		Left,
		Right,
		Front,
		Back
	};

	class SkyResource : public Resource, ISerializable
	{
	public:
		SkyResource(const std::string& path);
		SkyResource() = default;

		json Serialize() override;
		bool Deserialize(const json& j) override;

		void SetTextureFace(SkyFaces face, const std::string& path); 
		std::string GetFaceTexture(SkyFaces face) const;

		Ref<CubemapTexture> GetCubemap();

	private:
		std::string FrontTexturePath;
		std::string BackTexturePath;
		std::string LeftTexturePath;
		std::string RightTexturePath;
		std::string UpTexturePath;
		std::string DownTexturePath;


		Ref<CubemapTexture> cubemapTexture;

	private:
		void ResetCubemap();
	};
}

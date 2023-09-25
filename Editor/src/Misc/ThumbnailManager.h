#pragma once
#include "src/Core/Core.h"
#include "src/Rendering/Textures/Texture.h"
#include "src/Rendering/Buffers/Framebuffer.h"

class ThumbnailManager
{
private:
	std::unordered_map<std::string, Ref<Nuake::Texture>> m_Thumbnails;

	Ref<Nuake::FrameBuffer> m_Framebuffer;
	Ref<Nuake::FrameBuffer> m_ShadedFramebuffer;
	const Nuake::Vector2 m_ThumbnailSize = { 128, 128 };

public:
	ThumbnailManager();
	~ThumbnailManager() = default;

	static ThumbnailManager& Get();

	bool IsThumbnailLoaded(const std::string& path) const;
	Ref<Nuake::Texture> GetThumbnail(const std::string& path);
	void MarkThumbnailAsDirty(const std::string& path);
	Ref<Nuake::Texture> GenerateThumbnail(const std::string& path, Ref<Nuake::Texture> texture);
};
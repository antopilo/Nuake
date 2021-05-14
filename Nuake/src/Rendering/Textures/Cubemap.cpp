#include "Cubemap.h"
#include <GL\glew.h>
#include <iostream>
#include "HDR.h"

bool hasEnding(std::string const& fullString, std::string const& ending) {
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	}
	else {
		return false;
	}
}

CubemapTexture::CubemapTexture(const std::string& path) {
	m_RendererId = 0;
	m_FilePath = path;
	m_LocalBuffer = nullptr;
	m_Width = 0;
	m_Height = 0;
	m_BPP = 0;

	if (hasEnding(path, ".hdr")) {
		CreateFromHDR();
	}
	else {
		stbi_set_flip_vertically_on_load(0);

		GLvoid* texture_data[6];
		// Load file data.
		for (int face = 0; face < 6; face++)
			texture_data[face] = stbi_load((path + std::to_string(face) + ".jpg").c_str(), &m_Width, &m_Height, &m_BPP, 4);

		// Create cube map.
		glGenTextures(1, &m_RendererId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererId);

		// Set each face of the cube with image data.
		for (int face = 0; face < 6; face++) {

			GLenum target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face;
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
				0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data[face]
			);

			// Clear.
			if (texture_data[face])
				stbi_image_free(texture_data[face]);
			else
				std::cout << "Error: failed to load texture: " << path << std::endl;
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}
	
}


void CubemapTexture::CreateFromHDR() {

}

CubemapTexture::~CubemapTexture() {
	glDeleteTextures(1, &m_RendererId);
}

void CubemapTexture::Bind(unsigned int slot) const {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererId);
}

void CubemapTexture::Unbind() const {
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}


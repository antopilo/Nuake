#pragma once

#include "Nuake/Core/Maths.h"
#include <msdf-atlas-gen/msdf-atlas-gen.h>
#include <msdfgen/ext/import-font.h>

#include "Nuake/Rendering/Textures/Texture.h"

#include <string>
#include <memory>

using namespace Nuake;

namespace NuakeUI
{
    struct CharPos
    {
        double left;
        double right;
        double top;
        double bottom;
    };

    struct CharUV
    {
        Vector2 Pos;
        Vector2 Size;
    };

    class Char
    {
    private:
        unsigned int m_VBO;
        unsigned int m_VAO;

    public:
        unsigned int Unicode;
        float Advance;
        CharPos PlaneBounds;
        CharUV AtlasBounds;

        Char() {};
        Char(const unsigned int unicode, float advance, CharPos plane, CharUV atlas)
        {
            Unicode = unicode;
            Advance = advance;
            PlaneBounds = plane;
            AtlasBounds = atlas;
        }

        CharUV GetAtlasUV(const Vector2& atlasSize)
        {
            return AtlasBounds;
        }
    };

	class Font
	{
	public:
		Font(const std::string& path);
		Font() = default;
		~Font();

        float LineHeight = 0.f;

		static std::shared_ptr<Font> New(const std::string& path);

		msdfgen::FontHandle* GetFontHandle() const { return mFontHandle; }

        void AddChar(const unsigned int unicode, float advance, CharPos plane, CharUV atlas)
        {
            this->Chars[unicode] = Char(unicode, advance, plane, atlas);
        }

        Char GetChar(unsigned int unicode)
        {
            if (Chars.find(unicode) != Chars.end())
                return Chars[unicode];
            return Char();
        }

        std::shared_ptr<Texture> mAtlas;
	private:
		std::string mFilePath;
		msdfgen::FontHandle* mFontHandle;
		msdfgen::FreetypeHandle* mFreeTypeHandle;

        std::map<unsigned int, Char> Chars;
        

		bool Load(const std::string& path);
	};
}
#pragma once
#include "src/Core/Core.h"
#include <src/Vendors/msdfgen/ext/import-font.h>
#include <map>
#include <src/Rendering/Textures/Texture.h>
#include "src/Core/Maths.h"

namespace Nuake
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
    public:
        uint32_t Unicode;
        float Advance = 0.f;
        CharPos PlaneBounds;
        CharUV AtlasBounds = {};

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
    private:
        msdfgen::FreetypeHandle* ft;
        msdfgen::FontHandle* font;
        const char* fontFilename;

        std::map<unsigned int, Char> Chars;

    public:
        Ref<Texture> FontAtlas;

        Font() : ft(msdfgen::initializeFreetype()), font(nullptr), fontFilename(nullptr)
        {
            this->Chars = std::map<unsigned int, Char>();
        }

        ~Font() 
        {
            if (!ft)
                return;

            if (font)
                msdfgen::destroyFont(font);

            msdfgen::deinitializeFreetype(ft);
        }

        bool load(const char* fontFilename) 
        {
            if (ft && fontFilename) 
            {
                if (this->fontFilename && !strcmp(this->fontFilename, fontFilename))
                    return true;
                if (font)
                    msdfgen::destroyFont(font);
                if ((font = msdfgen::loadFont(ft, fontFilename))) 
                {
                    this->fontFilename = fontFilename;
                    return true;
                }

                this->fontFilename = nullptr;
            }
            return false;
        }

        operator msdfgen::FontHandle* () const 
        {
            return font;
        }

        msdfgen::FontHandle* GetFontHandle() { return font; }
        msdfgen::FreetypeHandle* GetFreetypeHandle() { return ft; }

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
    };
}

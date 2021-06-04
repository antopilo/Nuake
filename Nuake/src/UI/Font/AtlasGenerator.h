#pragma once
#include "msdf-atlas-gen/AtlasGenerator.h"
#include <src/Vendors/msdf-atlas-gen/FontGeometry.h>
#include <src/Vendors/msdf-atlas-gen/FontGeometry.h>



class FontGenerator
{
    template <typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GEN_FN>
    static bool makeAtlas(const std::vector<msdf_atlas::GlyphGeometry>& glyphs, const std::vector<msdf_atlas::FontGeometry>& fonts, const msdf_atlas::GeneratorAttributes& attr) {
        ImmediateAtlasGenerator<S, N, GEN_FN, BitmapAtlasStorage<T, N> > generator(config.width, config.height);
        generator.setAttributes(attr);
        generator.setThreadCount(1);
        generator.generate(glyphs.data(), glyphs.size());
        msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>) generator.atlasStorage();

        bool success = true;

        //if (config.imageFilename) {
        //    if (saveImage(bitmap, config.imageFormat, config.imageFilename, config.yDirection))
        //        puts("Atlas image file saved.");
        //    else {
        //        success = false;
        //        puts("Failed to save the atlas as an image file.");
        //    }
        //}

        return success;
    }
};
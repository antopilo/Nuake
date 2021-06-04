#pragma once
#include <src/UI/Font/Font.h>
#include <string>
#include "../Core/Core.h"
#include <src/Vendors/msdf-atlas-gen/Charset.h>
#include <src/Vendors/msdf-atlas-gen/AtlasGenerator.h>
#include <src/Vendors/msdf-atlas-gen/FontGeometry.h>
#include <src/Core/Logger.h>
#include <src/Vendors/msdf-atlas-gen/TightAtlasPacker.h>

#include <src/Vendors/msdf-atlas-gen/AtlasGenerator.h>
#include <src/Vendors/msdf-atlas-gen/ImmediateAtlasGenerator.h>
#include <src/Vendors/yoga/event/event.h>

#include "src/Vendors/msdf-atlas-gen/AtlasStorage.h"
#include "src/Vendors/msdf-atlas-gen/glyph-generators.h"
#include <src/Vendors/msdf-atlas-gen/BitmapAtlasStorage.h>
#include <src/Vendors/msdf-atlas-gen/image-save.h>

#include "../Rendering/Textures/Texture.h"
#include "msdf-atlas-gen/json-export.h"
typedef unsigned char byte;
struct Config
{
	msdf_atlas::ImageType imageType;
	msdf_atlas::ImageFormat imageFormat;
	msdf_atlas::YDirection yDirection;
	int width, height;
	double emSize;
	double pxRange;
	double angleThreshold;
	double miterLimit;
	void (*edgeColoring)(msdfgen::Shape&, double, unsigned long long);
	bool expensiveColoring;
	unsigned long long coloringSeed;
	msdf_atlas::GeneratorAttributes generatorAttributes;
	bool preprocessGeometry;
	bool kerning;
	int threadCount = 1;
};

class FontLoader
{
public:

	template <typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GEN_FN>
	static bool makeAtlas(const std::vector<msdf_atlas::GlyphGeometry>& glyphs,
							const std::vector<msdf_atlas::FontGeometry>& fonts,
							Config& config, Ref<Font> font) {
		// Create generator
		msdf_atlas::ImmediateAtlasGenerator<S, N, GEN_FN, msdf_atlas::BitmapAtlasStorage<T, N> > generator(config.width, config.height);

		// Setup generator settings
		generator.setAttributes(config.generatorAttributes);
		generator.setThreadCount(config.threadCount);
		generator.generate(glyphs.data(), glyphs.size());

		// Create bitmap
		msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>) generator.atlasStorage();

		// Create Texture from bitmap
		//msdf_atlas::exportJSON(fonts.data(), fonts.size(), config.emSize, config.pxRange, config.width, config.height, config.imageType, config.yDirection, "yayayayya.json", config.kerning);
		font->FontAtlas = CreateRef<Texture>(Vector2(config.width, config.height), bitmap, true);

		// Create Char structure
		return true;
	}
	static Ref<Font> LoadFont(const std::string path)
	{
		// create font
		Ref<Font> font = CreateRef<Font>();

		// Create atlas settings
		Config config{};
		config.pxRange = 2;
		config.emSize = 0.0;
		config.coloringSeed = 125155;
		config.imageType = msdf_atlas::ImageType::MTSDF;
		config.imageFormat = msdf_atlas::ImageFormat::UNSPECIFIED;
		config.yDirection = msdf_atlas::YDirection::BOTTOM_UP;
		config.edgeColoring = msdfgen::edgeColoringInkTrap;
		config.kerning = true;
		config.preprocessGeometry = false;
		config.angleThreshold = 3.0;
		config.miterLimit = 1.0;
		config.generatorAttributes.scanlinePass = true;
		config.generatorAttributes.config.overlapSupport = true;

		// Load file
		if (!font->load(path.c_str()))
			Logger::Log("Failed to laod font");
			
		// Load charset ASCII
		std::vector<msdf_atlas::GlyphGeometry> glyphs;
		std::vector<msdf_atlas::FontGeometry> fonts;
		msdf_atlas::FontGeometry fontGeometry(&glyphs);
		msdf_atlas::Charset charset = msdf_atlas::Charset::ASCII;

		// Load Create charset
		float fontScale = 32;
		bool preprocess = false;
		int loaded = fontGeometry.loadCharset(font->GetFontHandle(), fontScale, charset, config.preprocessGeometry, config.kerning);
		
		
		fonts.push_back(fontGeometry);

		if (glyphs.empty())
			Logger::Log("No glyphs loaded.");

		// Create atlas params
		msdf_atlas::TightAtlasPacker::DimensionsConstraint atlasSizeConstraint = msdf_atlas::TightAtlasPacker::DimensionsConstraint::MULTIPLE_OF_FOUR_SQUARE;
		msdf_atlas::TightAtlasPacker atlasPacker;
		atlasPacker.setDimensionsConstraint(atlasSizeConstraint);
		msdf_atlas::ImageType imageType = msdf_atlas::ImageType::MTSDF;
		atlasPacker.setPadding(imageType == msdf_atlas::ImageType::MSDF || imageType == msdf_atlas::ImageType::MTSDF ? 0 : -1);
		atlasPacker.setPixelRange(config.pxRange);
		atlasPacker.setUnitRange(config.emSize);
		atlasPacker.setMiterLimit(config.miterLimit);

		// Pack atlas
		if (int remaining = atlasPacker.pack(glyphs.data(), glyphs.size())) {
			if (remaining < 0) {
				Logger::Log("Failed to pack atlas.");
			}
			else {
				printf("Error: Could not fit %d out of %d glyphs into the atlas.\n", remaining, (int)glyphs.size());
				
			}
		}

		// update atlast size
		atlasPacker.getDimensions(config.width, config.height);
		if (!(config.width > 0 && config.height > 0))
			printf("Unable to determine atlas size.");

		config.emSize = atlasPacker.getScale();
		config.pxRange = atlasPacker.getPixelRange();

		// Color the glyph
		//unsigned long long glyphSeed = config.coloringSeed;
		//for (msdf_atlas::GlyphGeometry& glyph : glyphs) {
		//	glyphSeed *= 6364136223846793005ull;
		//	glyph.edgeColoring(config.edgeColoring, config.angleThreshold, glyphSeed);
		//}

		msdf_atlas::Workload([&glyphs, &config](int i, int threadNo) -> bool {
			unsigned long long glyphSeed = (6364136223846793005ull * (config.coloringSeed ^ i) + 1442695040888963407ull) * !!config.coloringSeed;
			glyphs[i].edgeColoring(config.edgeColoring, config.angleThreshold, glyphSeed);
			return true;
			}, glyphs.size()).finish(config.threadCount);

		// Create bitmap and char structure
		auto bitmap = makeAtlas<byte, float, 4, msdf_atlas::mtsdfGenerator>(glyphs, fonts, config, font);

		for (auto& g : glyphs)
		{
			CharPos plane = {};
			g.getQuadPlaneBounds(plane.left, plane.bottom, plane.right, plane.top);

			CharUV box = {};


			double x2, y2, z2, w2;
			g.getQuadAtlasBounds(x2, y2, z2, w2);
			box.Pos.x = x2;
			box.Pos.y = y2;
			box.Size.x = z2;
			box.Size.y = w2;
			font->AddChar(g.getCodepoint(), g.getAdvance(), plane, box);
		}

		return font;
	}
};
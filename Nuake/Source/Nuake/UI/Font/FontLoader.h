#pragma once
#include "Font.h"

#include <memory>
#include <string>

#include "Nuake/Rendering/Textures/Texture.h"

#include <Thirdparty/msdf-atlas-gen/msdfgen/core/BitmapRef.hpp>

typedef unsigned char byte;

namespace NuakeUI
{
	// Config used by the atlas generator.
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
		int threadCount = 2;
	};


	class FontLoader
	{
	public:
		static FontLoader& Get()
		{
			static FontLoader fontloader;
			return fontloader;
		}

		template <typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GEN_FN>
		static bool makeAtlas(const std::vector<msdf_atlas::GlyphGeometry>& glyphs,
			const std::vector<msdf_atlas::FontGeometry>& fonts,
			Config& config, std::shared_ptr<Font> font) {
			// Create generator
			msdf_atlas::ImmediateAtlasGenerator<S, N, GEN_FN, msdf_atlas::BitmapAtlasStorage<T, N> > generator(config.width, config.height);

			// Setup generator settings
			generator.setAttributes(config.generatorAttributes);
			generator.setThreadCount(config.threadCount);
			generator.generate(glyphs.data(), (int)glyphs.size());

			// Create bitmap
			msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>) generator.atlasStorage();

			TextureFlags flags;
			flags.minFilter = SamplerFilter::LINEAR;
			flags.magFilter = SamplerFilter::LINEAR;
			font->mAtlas = std::make_shared<Texture>(flags, Vector2(config.width, config.height), (void*)bitmap.pixels);

			// Create Char structure
			return true;
		}

		std::shared_ptr<Font> LoadFont(const std::string& path)
		{
			auto font = Font::New(path);

			// Create atlas settings
			Config config{};
			config.pxRange = 8.0;
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

			// Load charset ASCII
			std::vector<msdf_atlas::GlyphGeometry> glyphs;
			std::vector<msdf_atlas::FontGeometry> fonts;
			msdf_atlas::FontGeometry fontGeometry(&glyphs);
			msdf_atlas::Charset charset = msdf_atlas::Charset::ASCII;

			// Load Create charset
			float fontScale = 36.0f;

			bool preprocess = false;
			int loaded = fontGeometry.loadCharset(font->GetFontHandle(), fontScale, charset, config.preprocessGeometry, config.kerning);
			fonts.push_back(fontGeometry);

			if (glyphs.empty())
			{
				printf("Critical, Could not load font! \n");
			}

			// Create atlas params
			msdf_atlas::TightAtlasPacker::DimensionsConstraint atlasSizeConstraint = msdf_atlas::TightAtlasPacker::DimensionsConstraint::MULTIPLE_OF_FOUR_SQUARE;
			msdf_atlas::TightAtlasPacker atlasPacker;
			atlasPacker.setDimensionsConstraint(atlasSizeConstraint);
			msdf_atlas::ImageType imageType = msdf_atlas::ImageType::MTSDF;
			atlasPacker.setPadding(imageType == msdf_atlas::ImageType::MSDF || imageType == msdf_atlas::ImageType::MTSDF ? 0 : -1);
			atlasPacker.setPixelRange(config.pxRange);
			atlasPacker.setMiterLimit(config.miterLimit);

			// Pack atlas
			if (int remaining = atlasPacker.pack(glyphs.data(), (int)glyphs.size())) {
				if (remaining < 0) {
					printf("Critial - Failed to pack atlas");
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
				}, (int)glyphs.size()).finish(config.threadCount);

				// Create bitmap and char structure
				auto bitmap = makeAtlas<byte, float, 4, msdf_atlas::mtsdfGenerator>(glyphs, fonts, config, font);

				for (auto& g : glyphs)
				{
					CharPos plane = {};
					g.getQuadPlaneBounds(plane.left, plane.bottom, plane.right, plane.top);

					CharUV box = {};

					double x2, y2, z2, w2;
					g.getQuadAtlasBounds(x2, y2, z2, w2);
					box.Pos.x = (float)x2;
					box.Pos.y = (float)y2;
					box.Size.x = (float)z2;
					box.Size.y = (float)w2;
					font->AddChar(g.getCodepoint(), (float)g.getAdvance(), plane, box);
				}

				return font;
		
		}
	};
}
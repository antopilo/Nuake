#include "Font.h"
#include "../FileSystem.h"

namespace NuakeUI
{
	std::shared_ptr<Font> Font::New(const std::string& path)
	{
		return std::make_shared<Font>(path);
	}

	Font::Font(const std::string& path)
	{
		mFreeTypeHandle = msdfgen::initializeFreetype();
		Load(path);
	}

	Font::~Font()
	{
		msdfgen::destroyFont(mFontHandle);
	}

	bool Font::Load(const std::string& path)
	{
		bool exists = FileSystem::FileExists(path);
		mFontHandle = msdfgen::loadFont(mFreeTypeHandle, path.c_str());
		if (!mFontHandle)
			return false;

		msdfgen::FontMetrics metrics;
		msdfgen::getFontMetrics(metrics, mFontHandle);

		LineHeight = (float)metrics.lineHeight;
		return true;
	}
}
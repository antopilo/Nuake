#include "Stylesheet.h"

namespace Nuake {
	namespace UI {
		Ref<StyleSheet> StyleSheet::New(const std::string& path)
		{
			return CreateRef<StyleSheet>(path);
		}
	}
}
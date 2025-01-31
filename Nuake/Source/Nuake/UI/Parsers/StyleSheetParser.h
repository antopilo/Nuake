#pragma once
#include "../Styles/StyleSheet.h"

#include <string>
#include <memory>

#include "../Vendors/katana-parser/katana.h"

#include "Nuake/UI/Nodes/Canvas.h"

namespace NuakeUI
{
	class StyleSheetParser
	{
	public:
		static StyleSheetParser& Get()
		{
			static StyleSheetParser parser;
			return parser;
		}

		StyleSheetParser() = default;
		~StyleSheetParser() = default;

		std::shared_ptr<StyleSheet> Parse(Ref<NuakeUI::Canvas> canvas, const std::string& path);

	private:
		Ref<Canvas> currentCanvas;
		std::string _parsingPath;
		std::vector<std::string> _visitedFiles;

		bool FileAlreadyVisited(const std::string& path);

		void ParseRules(KatanaStylesheet* katanaStylesheet, StyleSheetPtr stylesheet);
		void ParseImportRule(KatanaImportRule* rule, StyleSheetPtr styleSheet);
		void ParseStyleRule(KatanaRule* rule, StyleSheetPtr stylesheet);
	};
}
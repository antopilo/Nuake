#pragma once
#include <string>

#include "katana-parser/katana.h"

#include "../Core/FileSystem.h"
#include "../Core/Logger.h"
#include "../Core/Core.h"
namespace UI
{
	class StyleSheet
	{
	private:
		KatanaOutput* Data;
	public:
		std::string Path;


		StyleSheet(const std::string& path)
		{
			this->Path = path;
			ParseSheet();
		}

		static Ref<StyleSheet> New(const std::string& path);

		~StyleSheet()
		{
			katana_destroy_output(Data);
		}

		bool ParseSheet()
		{
			std::string content = FileSystem::ReadFile(this->Path);

			Data = katana_parse(content.c_str(), content.length(), KatanaParserModeStylesheet);

			if (Data->errors.length > 0)
			{
				KatanaArray errors = Data->errors;
				return false;
			}
			//Data->stylesheet->rules
			katana_dump_output(Data);
			return true;
		}

		
	};
}
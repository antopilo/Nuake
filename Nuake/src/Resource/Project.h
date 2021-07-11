#pragma once
#include <string>
#include "../Core/Core.h"
#include "../Scene/Scene.h"
#include "Serializable.h"

#include "FGD/FGDFile.h"

namespace Nuake
{
	class Project : public ISerializable
	{
	public:
		std::string Name;
		std::string Description;
		std::string FullPath;

		// Path of the trenchbroom .exe folder
		std::string TrenchbroomPath = "";

		Ref<Scene> DefaultScene;
		Ref<FGDFile> EntityDefinitionsFile;

		Project(const std::string Name, const std::string Description, const std::string& FullPath, const std::string& defaultScenePath = "");
		Project();

		void Save();
		void SaveAs(const std::string FullPath);

		static Ref<Project> New(const std::string Name, const std::string Description, const std::string FullPath);
		static Ref<Project> New();
		static Ref<Project> Load(std::string path);

		json Serialize() override;
		bool Deserialize(const std::string& str) override;
	};
}

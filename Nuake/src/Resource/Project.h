#pragma once
#include <string>
#include "../Core/Core.h"
#include "../Scene/Scene.h"
#include "Serializable.h"
class Project : public ISerializable
{
public:
	std::string Name;
	std::string Description;
	std::string FullPath;

	Ref<Scene> DefaultScene;

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
#pragma once
#include <src/Resource/Serializable.h>
#include <string>
#include <vector>

namespace Nuake
{
	class ProjectPreview : ISerializable
	{
	public:
		std::string Name;
		std::string Path;
		std::string Description;


		ProjectPreview(const std::string& path);
		ProjectPreview() = default;
		~ProjectPreview() = default;

		json Serialize() override;
		bool Deserialize(const std::string& data) override;
	private:
		void ReadProjectFile();
	};

	class EditorInterface;
	class WelcomeWindow
	{
	private:
		const std::string _RecentProjectFilePath = "recent.json";
		const std::string _RecentProjectFileDefaultContent = "{ \"Projects\": [ ] }";
		std::vector<ProjectPreview> _Projects;
		uint32_t SelectedProject = 0;
		EditorInterface* _Editor;
	public:

		WelcomeWindow(Nuake::EditorInterface* editor);
		~WelcomeWindow() = default;

		void Draw();

	private:
		void ParseRecentFile();
		void SaveRecentFile();
	};
}

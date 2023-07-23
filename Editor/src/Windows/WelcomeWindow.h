#pragma once
#include <src/Resource/Serializable.h>
#include <src/Core/Core.h>

#include <string>
#include <vector>

namespace Nuake
{
	class Texture;
	class ProjectPreview : ISerializable
	{
	public:
		std::string Name;
		std::string Path;
		std::string Description;

		Ref<Texture> ProjectIcon;

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
		const std::string NUAKE_LOGO_PATH = "resources/Images/logo_white.png";
		Ref<Texture> _NuakeLogo;
		EditorInterface* _Editor;

		const std::string _RecentProjectFilePath = "recent.json";
		const std::string _RecentProjectFileDefaultContent = "{ \"Projects\": [ ] }";
		
		uint32_t SelectedProject = 0;
		std::vector<ProjectPreview> _Projects;
		std::string queuedProjectPath;
	public:
		WelcomeWindow(Nuake::EditorInterface* editor);
		~WelcomeWindow() = default;

		void Draw();

		bool IsProjectQueued() const { return !queuedProjectPath.empty(); };
		void LoadQueuedProject();
	private:
		void DrawRecentProjectsSection();
		void DrawProjectItem(const uint32_t projectPreview);
		void DrawRightControls();

		void ParseRecentFile();
		void SaveRecentFile();
	};
}

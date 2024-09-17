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
		bool Deserialize(const json& j) override;

	private:
		void ReadProjectFile();
	};

	class EditorInterface;
	class WelcomeWindow
	{
	private:
		const std::string NUAKE_LOGO_PATH = "Resources/Images/logo_white.png";
		Ref<Texture> _NuakeLogo;
		EditorInterface* _Editor;

		std::string _RecentProjectFilePath = "";
		const std::string _RecentProjectFileDefaultContent = "{ \"Projects\": [ ] }";
		
		int32_t SelectedProject = -1;
		std::vector<ProjectPreview> _Projects;
		std::string queuedProjectPath;
		std::string queuedRemovalPath;

	public:
		WelcomeWindow(Nuake::EditorInterface* editor);
		~WelcomeWindow() = default;

		void Draw();

		bool IsProjectQueued() const { return !queuedProjectPath.empty(); };
		void LoadQueuedProject();

		void ImportProject(const std::string& file);

	private:
		void DrawRecentProjectsSection();
		void DrawProjectItem(const uint32_t projectPreview);
		void DrawRightControls();

		void ParseRecentFile();
		void SaveRecentFile();
	};
}

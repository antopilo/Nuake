#pragma once
#include <string>

namespace Nuake
{
	class EditorInterface;

	class NewProjectWindow
	{
	private:
		uint32_t m_SelectedTemplate = 0;
		bool m_HasCreatedProject = false;
		EditorInterface* m_Editor;

	public:
		NewProjectWindow(EditorInterface* editor);
		~NewProjectWindow() = default;

		void Draw();

		void DrawProjectTemplate(uint32_t i, const std::string& title, const std::string& description);

		bool HasCreatedProject() const
		{
			return m_HasCreatedProject;
		}
	};
}

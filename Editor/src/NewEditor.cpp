#include "NewEditor.h"
#include "Engine.h"
#include "src/Core/Input.h"
#include <dependencies/glfw/include/GLFW/glfw3.h>

namespace Nuake
{
	NewEditor::NewEditor()
	{
		m_UserInterface = UI::UserInterface::New("Editor", "resources/Interface/Editor.interface");
		FetchNodes();
	}

	void NewEditor::FetchNodes()
	{
		m_SceneFramebuffer = Engine::GetCurrentWindow()->GetFrameBuffer();
		m_ViewportNode = m_UserInterface->GetNodeByID("viewport-texture");
		m_ViewportNode->BackgroundTexture = m_SceneFramebuffer->GetTexture();

		m_FileMenu = m_UserInterface->GetNodeByID("action-menu-file");

		m_Splitter = m_UserInterface->GetNodeByID("splitter");
		m_Splitter2 = m_UserInterface->GetNodeByID("splitter2");

		m_Right = m_UserInterface->GetNodeByID("right");
		m_Left = m_UserInterface->GetNodeByID("left");
		m_Inspector = m_UserInterface->GetNodeByID("inspector");
	}

	bool dragging = false;
	bool dragging2 = false;
	void NewEditor::Update(Timestep ts)
	{
		if (Input::IsKeyPressed(GLFW_KEY_F1))
		{
			m_UserInterface->Reload();
			FetchNodes();
		}
		
		if (m_FileMenu->IsClicked)
		{
			// Parse the project and load it.
			std::string projectPath = FileDialog::OpenFile(".project");

			FileSystem::SetRootDirectory(projectPath + "/../");
			Ref<Project> project = Project::New();
			if (!project->Deserialize(FileSystem::ReadFile(projectPath, true)))
			{
				Logger::Log("Error loading project: " + projectPath, CRITICAL);
				return;
			}

			project->FullPath = projectPath;
			Engine::LoadProject(project);
		}
		float mouseX = Input::GetMouseX();
		if (m_Splitter->IsHover && Input::IsMouseButtonDown(0))
		{
			dragging = true;
		}

		if (m_Splitter2->IsHover && Input::IsMouseButtonDown(0))
			dragging2 = true;

		if (dragging)
		{
			if (!Input::IsMouseButtonDown(0))
				dragging = false;

			float totalWidth = m_UserInterface->Size.x;

			float mouseX = Input::GetMouseX();
			float ratio = mouseX / totalWidth;

			m_Inspector->NormalStyle.Width.Unit = Layout::PIXEL;
			m_Inspector->NormalStyle.Width.Value = (1.0f - ratio) * totalWidth - 3.0f;

			m_ViewportNode->NormalStyle.Width.Unit = Layout::PIXEL;
			m_ViewportNode->NormalStyle.Width.Value = ratio * totalWidth;

			m_Inspector->HoverStyle.Width.Unit = Layout::PIXEL;
			m_Inspector->HoverStyle.Width.Value = (1.0f - ratio) * totalWidth - 3.0f;

			m_ViewportNode->HoverStyle.Width.Unit = Layout::PIXEL;
			m_ViewportNode->HoverStyle.Width.Value = ratio * totalWidth;

			Logger::Log("mouseX " + std::to_string(ratio));
		}

		if (dragging2 == true)
		{
			if (!Input::IsMouseButtonDown(0))
				dragging2 = false;

			float totalWidth = m_UserInterface->Size.x;
			float mouseX = Input::GetMouseX();
			float ratio = mouseX / totalWidth;

		}

		m_UserInterface->Update(ts);

		// Layout logic
	}

	void NewEditor::Draw(Vector2 screenSize)
	{
		m_UserInterface->Draw(screenSize);
	}
}
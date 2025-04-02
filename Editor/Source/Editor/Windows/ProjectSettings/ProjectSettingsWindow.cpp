#include "ProjectSettingsWindow.h"
#include <imgui/imgui.h>
#include "../../Misc/InterfaceFonts.h"
#include <Nuake/FileSystem/FileDialog.h>
#include "Nuake/FileSystem/FileSystem.h"
#include "../EditorInterface.h"
#include "../../Commands/Commands/Commands.h"
#include <Nuake/Audio/AudioManager.h>
#include <Nuake/Modules/ModuleDB.h>

ProjectSettingsCategoryWindowGeneral::ProjectSettingsCategoryWindowGeneral(Ref<Nuake::Project> project) :
	m_Project(project)
{
	Name = "General";
}

void ProjectSettingsCategoryWindowGeneral::Draw()
{
	ImGui::InputText("Project Name", &m_Project->Name);

	ImGui::InputTextMultiline("Project Description", &m_Project->Description);
	
	if (ImGui::Button("Locate"))
	{
		const std::string& locationPath = Nuake::FileDialog::OpenFile("TrenchBroom (.exe)\0TrenchBroom.exe\0");

		if (!locationPath.empty())
		{
			m_Project->TrenchbroomPath = locationPath;
		}
	}

	ImGui::SameLine();
    ImGui::InputText("Trenchbroom Path", &m_Project->TrenchbroomPath);

	ImGui::ColorEdit4("Primary Color", &m_Project->Settings.PrimaryColor.r);
}

ProjectSettingsWindow::ProjectSettingsWindow()
{

}

void ProjectSettingsWindow::Init(Ref<Nuake::Project> project)
{
    if (m_Project == project)
    {
        return;
    }

    m_Project = project;

    m_CategoryWindows.clear();
    m_CategoryWindows.push_back(CreateRef<ProjectSettingsCategoryWindowGeneral>(project));
    m_CategoryWindows.push_back(CreateRef<ProjectSettingsCategoryWindowViewport>(project));
    m_CategoryWindows.push_back(CreateRef<ProjectSettingsCategoryWindowRendering>(project));
    m_CategoryWindows.push_back(CreateRef<ProjectSettingsCategoryScripting>(project));
    m_CategoryWindows.push_back(CreateRef<ProjectSettingsCategoryPhysics>(project));
    m_CategoryWindows.push_back(CreateRef<ProjectSettingsCategoryAudio>(project));

    for (auto& m : ModuleDB::Get().GetModules())
    {
        m_CategoryWindows.push_back(CreateRef<ProjectSettingsModuleWindow>(m));
    }
}

void ProjectSettingsWindow::Draw()
{
    if (!m_DisplayWindow)
    {
        return;
    }

    ImGui::SetNextWindowSizeConstraints({800, 400}, {1280, 720});
    if (ImGui::Begin("Project Settings", &m_DisplayWindow, ImGuiWindowFlags_NoDocking))
    {
        ImVec4* colors = ImGui::GetStyle().Colors;
        ImGui::PushStyleColor(ImGuiCol_ChildBg, colors[ImGuiCol_TitleBgCollapsed]);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 8);
        if(ImGui::BeginChild("ProjectSettingsLeft", { 200, ImGui::GetContentRegionAvail().y }, true))
        {
            for (int i = 0; i < static_cast<int>(m_CategoryWindows.size()); i++)
            {
                ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding;
                bool is_selected = m_CurrentTab == i;
                if (is_selected)
                {
                    base_flags |= ImGuiTreeNodeFlags_Selected;
                }

                Ref<ProjectSettingsCategoryWindow> window = m_CategoryWindows.at(i);

                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
                ImGui::PushFont(FontManager::GetFont(Bold));
                ImGui::TreeNodeEx(window->Name.c_str(), base_flags);

                if (ImGui::IsItemClicked())
                {
                    m_CurrentTab = i;
                }

                ImGui::TreePop();

                ImGui::PopFont();
                ImGui::PopStyleVar();
            }
        }
        ImGui::EndChild();

        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        ImGui::SameLine();

        if(ImGui::BeginChild("ProjectSettingsRight", ImGui::GetContentRegionAvail(), ImGuiChildFlags_Border))
        {
            m_CategoryWindows.at(m_CurrentTab)->Draw();
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

ProjectSettingsCategoryWindowViewport::ProjectSettingsCategoryWindowViewport(Ref<Nuake::Project> project) :
    m_Project(project)
{
    Name = "Viewport";
}

void ProjectSettingsCategoryWindowViewport::Draw()
{
    ImGui::DragFloat("Outline Radius", &m_Project->Settings.OutlineRadius, 0.1f, 1.0f, 90.0f);
    ImGui::DragFloat("Gizmo Size", &m_Project->Settings.GizmoSize, 0.01f, 0.05f, 0.5f);
    ImGui::Separator();
    ImGui::Checkbox("Smooth Camera", &m_Project->Settings.SmoothCamera);

    if (!m_Project->Settings.SmoothCamera)
    {
        ImGui::BeginDisabled();
    }

    ImGui::DragFloat("Smooth Camera Speed", &m_Project->Settings.SmoothCameraSpeed, 0.01f, 0.01f, 1.0f);

    if (!m_Project->Settings.SmoothCamera)
    {
        ImGui::EndDisabled();
    }
}

ProjectSettingsCategoryWindowRendering::ProjectSettingsCategoryWindowRendering(Ref<Nuake::Project> project) :
    m_Project(project)
{
    Name = "Rendering";
}

void ProjectSettingsCategoryWindowRendering::Draw()
{
    bool oldVsync = m_Project->Settings.VSync;
    ImGui::Checkbox("VSync", &m_Project->Settings.VSync);
    if (oldVsync != m_Project->Settings.VSync)
    {
        Nuake::EditorInterface::PushCommand(SetVSync(m_Project->Settings.VSync));
    }

    ImGui::DragFloat("Resolution Scale", &m_Project->Settings.ResolutionScale, 0.01f, 0.05f, 2.0f);
}

ProjectSettingsCategoryScripting::ProjectSettingsCategoryScripting(Ref<Nuake::Project> project) :
    m_Project(project)
{
    Name = "Scripting";
}

void ProjectSettingsCategoryScripting::Draw()
{
}

ProjectSettingsCategoryPhysics::ProjectSettingsCategoryPhysics(Ref<Nuake::Project> project) :
    m_Project(project)
{
    Name = "Physics";
}

void ProjectSettingsCategoryPhysics::Draw()
{
    int oldStep = Engine::GetProject()->Settings.PhysicsStep;
    ImGui::DragInt("Physics Step", &Engine::GetProject()->Settings.PhysicsStep, 1.0f, 30.0f, 200.0f);
    if (oldStep != Engine::GetProject()->Settings.PhysicsStep)
    {
        Engine::SetPhysicsStep(Engine::GetProject()->Settings.PhysicsStep);
    }

    ImGui::DragInt("Maximum SubSteps", &Engine::GetProject()->Settings.MaxPhysicsSubStep, 1, 8, 128);

    ImGui::DragInt("Maximum Bodies", &Engine::GetProject()->Settings.MaxPhysicsBodies, 1.0f, 256, 8000);
    ImGui::DragInt("Maximum Body Pair", &Engine::GetProject()->Settings.MaxPhysicsBodyPair, 1.0f, 256, 4096);
    ImGui::DragInt("Maximum Contact Constraint", &Engine::GetProject()->Settings.MaxPhysicsContactConstraints, 1.0f, 256, 4096);
}

ProjectSettingsCategoryAudio::ProjectSettingsCategoryAudio(Ref<Nuake::Project> project) :
    m_Project(project)
{
    Name = "Audio";
}

ProjectSettingsModuleWindow::ProjectSettingsModuleWindow(const std::string& inModuleName) :
    moduleName(inModuleName)
{
    Name = moduleName;
}

void ProjectSettingsModuleWindow::Draw()
{
    auto meta = entt::resolve(entt::hashed_string(Name.c_str()));
    for (auto [id, data] : meta.data())
    {
        auto propDisplayName = data.prop(HashedName::DisplayName);
        if (propDisplayName)
        {
            auto propVal = propDisplayName.value();
            const char* settingName = *propVal.try_cast<const char*>();

            ImGui::Text(settingName);
        }
    }

	for (auto [id, func] : meta.func())
	{
		auto propDisplayName = func.prop(HashedName::DisplayName);
        
        if (propDisplayName)
        {
            auto propVal = propDisplayName.value();
            const char* settingName = *propVal.try_cast<const char*>();

            ImGui::Text(settingName);
        }


        // Setting
		//std::string funcName = "UnknownFunc";
		//if (propDisplayName)
		//{
		//	funcName = std::string(*propDisplayName.value().try_cast<const char*>());
		//}
		//
		//std::string msg = std::string(returnType) + " " + std::string(funcName) + "(";
		//std::vector<std::string_view> args;
        //
		//if (func.arity() > 0)
		//{
		//	auto propArgsName = func.prop(HashedName::ArgsName);
		//	if (propArgsName)
		//	{
		//		std::vector<std::string> argsName = *propArgsName.value().try_cast<std::vector<std::string>>();
		//		for (int i = 0; i < func.arity(); i++)
		//		{
		//			const std::string argType = std::string(func.arg(i).info().name());
		//			args.push_back(argType);
        //
		//			msg += argType + " " + argsName[i];
        //
		//			if (i < func.arity() - 1)
		//			{
		//				msg += ", ";
		//			}
		//		}
		//	}
		//}
        //
		//
		//msg += ")";
	    //
		//Logger::Log(msg, "", VERBOSE);
	}
}

void ProjectSettingsCategoryAudio::Draw()
{
    float oldVolume = Engine::GetProject()->Settings.GlobalVolume;
    ImGui::DragFloat("Global Volume", &Engine::GetProject()->Settings.GlobalVolume, 0.01f, 0.0f, 1.0f);

    if (oldVolume != Engine::GetProject()->Settings.GlobalVolume)
    {
        Nuake::AudioManager::Get().SetGlobalVolume(Engine::GetProject()->Settings.GlobalVolume);
    }

    ImGui::DragInt("Max Active Voices", &Engine::GetProject()->Settings.MaxActiveVoiceCount, 1.0f, 8.0f, 64.0f);
}

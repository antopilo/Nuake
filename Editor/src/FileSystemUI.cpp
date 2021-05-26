#include "FileSystemUI.h"
#include <src/Vendors/imgui/imgui.h>
#include <src/Resource/FontAwesome5.h>
#include <src/Scene/Entities/Components/ParentComponent.h>
#include <src/Rendering/Textures/Texture.h>
#include <src/Core/TextureManager.h>
#include "EditorInterface.h"

// TODO: add filetree in same panel
void FileSystemUI::Draw()
{
    Ref<Directory> rootDirectory = FileSystem::GetFileTree();
    if (!rootDirectory)
        return;
    if (ImGui::Begin("Tree browser"))
    {
        if (ImGui::BeginPopupContextItem("item context menu"))
        {
            if (ImGui::Selectable("Set to zero"));
            if (ImGui::Selectable("Set to PI"));
            ImGui::SetNextItemWidth(-1);
            ImGui::EndPopup();
        }
        ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
        bool is_selected = m_CurrentDirectory == FileSystem::RootDirectory;
        if (is_selected)
            base_flags |= ImGuiTreeNodeFlags_Selected;
        std::string icon = ICON_FA_FOLDER;

        bool open = ImGui::TreeNodeEx((icon + " " + rootDirectory->name).c_str(), base_flags);
        if (ImGui::IsItemClicked())
        {
            m_CurrentDirectory = FileSystem::RootDirectory;
        }
        if (open)
        {
            //EditorInterfaceDrawFiletree(rootDirectory);
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void FileSystemUI::DrawDirectoryContent()
{
}

void FileSystemUI::DrawFiletree()
{
}


void FileSystemUI::EditorInterfaceDrawFiletree(Ref<Directory> dir)
{
    for (auto d : dir->Directories)
    {
        ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
        bool is_selected = m_CurrentDirectory == d;
        if (is_selected)
            base_flags |= ImGuiTreeNodeFlags_Selected;

        if (d->Directories.size() == 0)
        {
            base_flags = ImGuiTreeNodeFlags_Leaf;
        }
        std::string icon = ICON_FA_FOLDER;
        if (is_selected)
            icon = ICON_FA_FOLDER_OPEN;
        bool open = ImGui::TreeNodeEx((icon + " " + d->name).c_str(), base_flags);

        if (ImGui::IsItemClicked())
            m_CurrentDirectory = d;
        if (open)
        {
            if (d->Directories.size() > 0)
                EditorInterfaceDrawFiletree(d);
            ImGui::TreePop();
        }

    }
}



void FileSystemUI::DrawDirectory(Ref<Directory> directory)
{
    ImGui::PushFont(EditorInterface::bigIconFont);
    std::string id = ICON_FA_FOLDER + std::string("##") + directory->name;
    if (ImGui::Button(id.c_str(), ImVec2(100, 100)))
        m_CurrentDirectory = directory;
    ImGui::Text(directory->name.c_str());
    ImGui::PopFont();
}

bool FileSystemUI::EntityContainsItself(Entity source, Entity target)
{
    ParentComponent& targeParentComponent = target.GetComponent<ParentComponent>();
    if (!targeParentComponent.HasParent)
        return false;

    Entity currentParent = target.GetComponent<ParentComponent>().Parent;
    while (currentParent != source)
    {
        if (currentParent.GetComponent<ParentComponent>().HasParent)
            currentParent = currentParent.GetComponent<ParentComponent>().Parent;
        else
            return false;

        if (currentParent == source)
            return true;
    }
    return true;
}

void FileSystemUI::DrawFile(Ref<File> file)
{
    ImGui::PushFont(EditorInterface::bigIconFont);
    if (file->Type == ".png" || file->Type == ".jpg")
    {
        Ref<Texture> texture = TextureManager::Get()->GetTexture(file->fullPath);
        ImGui::ImageButton((void*)texture->GetID(), ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
    }
    else
    {
        const char* icon = ICON_FA_FILE;
        if (file->Type == ".shader")
            icon = ICON_FA_FILE_CODE;
        if (file->Type == ".map")
            icon = ICON_FA_BROOM;
        if (file->Type == ".ogg" || file->Type == ".mp3" || file->Type == ".wav" || file->Type == ".flac")
            icon = ICON_FA_FILE_AUDIO;
        if (file->Type == ".cpp" || file->Type == ".h" || file->Type == ".cs" || file->Type == ".py" || file->Type == ".lua")
            icon = ICON_FA_FILE_CODE;
        if (ImGui::Button(icon, ImVec2(100, 100)))
        {
            if (ImGui::BeginPopupContextItem("item context menu"))
            {
                if (ImGui::Selectable("Set to zero"));
                if (ImGui::Selectable("Set to PI"));
                ImGui::EndPopup();
            }
        }
    }
    ImGui::Text(file->name.c_str());
    ImGui::PopFont();

}

void FileSystemUI::DrawDirectoryExplorer()
{
    if (ImGui::Begin("File browser"))
    {
        // Wrapping.
        int width = ImGui::GetWindowWidth();
        ImVec2 buttonSize = ImVec2(100, 100);
        int amount = (width / 100); // -2 because button overflow width + ... button.
        int i = 1; // current amount of item per row.
        if (ImGui::BeginTable("ssss", amount))
        {
            // Button to go up a level.
            if (m_CurrentDirectory != FileSystem::RootDirectory)
            {
                ImGui::TableNextColumn();
                if (ImGui::Button("..", ImVec2(100, 100)))
                    m_CurrentDirectory = m_CurrentDirectory->Parent;
                ImGui::TableNextColumn();
                // Increment item per row tracker.
                i++;
            }

            // Exit if no current directory.
            if (!m_CurrentDirectory) {
                ImGui::EndTable();
                ImGui::End();
                return;
            }

            if (m_CurrentDirectory && m_CurrentDirectory->Directories.size() > 0)
            {
                for (auto d : m_CurrentDirectory->Directories)
                {
                    DrawDirectory(d);
                    if (i - 1 % amount != 0)
                        ImGui::TableNextColumn();
                    else
                        ImGui::TableNextRow();
                    i++;
                }
            }
            if (m_CurrentDirectory && m_CurrentDirectory->Files.size() > 0)
            {
                for (auto f : m_CurrentDirectory->Files)
                {
                    DrawFile(f);
                    if (i - 1 % amount != 0)
                        ImGui::TableNextColumn();
                    else
                        ImGui::TableNextRow();
                    i++;
                }
            }

            ImGui::EndTable();
        }


    }
    ImGui::End();
}


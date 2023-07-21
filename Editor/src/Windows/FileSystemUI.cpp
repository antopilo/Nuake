#include "FileSystemUI.h"

#include <src/Vendors/imgui/imgui.h>

#include <src/Vendors/imgui/imgui_internal.h>
#include "src/Resource/FontAwesome5.h"
#include "src/Scene/Components/ParentComponent.h"
#include "src/Rendering/Textures/Texture.h"
#include "src/Rendering/Textures/TextureManager.h"
#include "EditorInterface.h"

const std::string TEMPLATE_SCRIPT_FIRST = R"(import "Nuake:Engine" for Engine 
import "Nuake:ScriptableEntity" for ScriptableEntity 
import "Nuake:Input" for Input 
import "Nuake:Math" for Vector3, Math 
import "Nuake:Scene" for Scene 

class )";

const std::string TEMPLATE_SCRIPT_SECOND = R"( is ScriptableEntity { 
        construct new() { 
        } 

        // Called when the scene gets initialized 
        init() { 
            // Engine.Log("Hello World!") 
        } 
 
        // Called every update 
        update(ts) { 
        } 
 
        // Called 90 times per second 
        fixedUpdate(ts) { 
        }  
         
        // Called on shutdown 
        exit() { 
        } 
} 
)";

#include <src/Rendering/Textures/Material.h>

namespace Nuake
{
    
    // TODO: add filetree in same panel
    void FileSystemUI::Draw()
    {

    }

    void FileSystemUI::DrawDirectoryContent()
    {
    }

    void FileSystemUI::DrawFiletree()
    {
    }

    void FileSystemUI::EditorInterfaceDrawFiletree(Ref<Directory> dir)
    {
        ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_FramePadding;
        //if (is_selected) 
       
        if (m_CurrentDirectory == dir)
            base_flags |= ImGuiTreeNodeFlags_Selected;
        if (dir->Directories.size() <= 0)
            base_flags |= ImGuiTreeNodeFlags_Leaf;

        std::string icon = ICON_FA_FOLDER;
        bool open = ImGui::TreeNodeEx((icon + "  " + dir->name.c_str()).c_str(), base_flags);

        if (ImGui::IsItemClicked())
            m_CurrentDirectory = dir;

        if (open)
        {
            if (dir->Directories.size() > 0)
                for (auto& d : dir->Directories)
                    EditorInterfaceDrawFiletree(d);
            ImGui::TreePop();
        }
    }

    void FileSystemUI::DrawDirectory(Ref<Directory> directory)
    {
        ImGui::PushFont(FontManager::GetFont(Icons));
        const char* icon = ICON_FA_FOLDER;
        const std::string id = ICON_FA_FOLDER + std::string("##") + directory->name;
        if (ImGui::Button(id.c_str(), ImVec2(100, 100)))
        {
            m_CurrentDirectory = directory;
        }

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

    void FileSystemUI::DrawFile(Ref<File> file, uint32_t drawId)
    {
        ImGui::PushFont(EditorInterface::bigIconFont);
        std::string fileExtension = file->GetExtension();
        if (fileExtension == ".png" || fileExtension == ".jpg")
        {
            Ref<Texture> texture = TextureManager::Get()->GetTexture(file->GetAbsolutePath());
            ImGui::ImageButton((void*)texture->GetID(), ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
        }
        else
        {
            const char* icon = ICON_FA_FILE;
            if (fileExtension == ".shader" || fileExtension == ".wren")
                icon = ICON_FA_FILE_CODE;
            if (fileExtension == ".map")
                icon = ICON_FA_BROOM;
            if (fileExtension == ".ogg" || fileExtension == ".mp3" || fileExtension == ".wav")
                icon = ICON_FA_FILE_AUDIO;
            if (fileExtension == ".md3" || fileExtension == ".obj")
                icon = ICON_FA_FILE_IMAGE;

            std::string fullName = icon + std::string("##") + file->GetAbsolutePath();
            if (ImGui::Button(fullName.c_str(), ImVec2(100, 100)))
            {
                Editor->Selection = EditorSelection(file);
            }

            if (ImGui::BeginDragDropSource())
            {
                char pathBuffer[256];
                std::strncpy(pathBuffer, file->GetAbsolutePath().c_str(), sizeof(pathBuffer));
                std::string dragType;
                if (fileExtension == ".wren")
                    dragType = "_Script";
                else if (fileExtension == ".map")
                    dragType = "_Map";
                else if (fileExtension == ".obj" || fileExtension == ".mdl" || fileExtension == ".gltf" || fileExtension == ".md3" || fileExtension == ".fbx")
                    dragType = "_Model";
                else if (fileExtension == ".interface")
                    dragType = "_Interface";
                else if (fileExtension == ".prefab")
                    dragType = "_Prefab";

                ImGui::SetDragDropPayload(dragType.c_str(), (void*)(pathBuffer), sizeof(pathBuffer));
                ImGui::Text(file->GetName().c_str());
                ImGui::EndDragDropSource();
            }
        }

        const std::string hoverMenuId = std::string("item_hover_menu") + std::to_string(drawId);
        if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(1))
        {
            ImGui::OpenPopup(hoverMenuId.c_str());
            m_hasClickedOnFile = true;
        }
        
        if (ImGui::BeginPopup(hoverMenuId.c_str()))
        {
            if (ImGui::MenuItem("Show in File Explorer"))
            {
                OS::ShowInFileExplorer(file->GetAbsolutePath());
            }

            if(file->GetExtension() == ".wren")
            {
                ImGui::Separator();

                if(ImGui::MenuItem("Open..."))
                {
                    OS::OpenIn(file->GetAbsolutePath());
                }
            }

            if(file->GetExtension() != ".project")
            {
                ImGui::Separator();
            
                if (ImGui::MenuItem("Delete"))
                {
                    if(FileSystem::RemoveFile(file->GetAbsolutePath()) != 0)
                    {
                        Logger::Log("Failed to remove file: " + file->GetRelativePath(), CRITICAL);
                    }
                    RefreshFileBrowser();
                }
            }
            
            ImGui::EndPopup();
        }

        ImGui::Text(file->GetName().c_str());
        ImGui::PopFont();
    }

    bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f)
    {
        using namespace ImGui;
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImGuiID id = window->GetID("##Splitter");
        ImRect bb;
        bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
        bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
        return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
    }

    void FileSystemUI::DrawContextMenu()
    {
        if (!m_hasClickedOnFile && ImGui::IsMouseReleased(1) && ImGui::IsWindowHovered())
        {
            ImGui::OpenPopup("window_hover_menu");
        }
        
        if (ImGui::BeginPopup("window_hover_menu"))
		{
			if (ImGui::MenuItem("New folder"))
			{

			}

			if (ImGui::MenuItem("New Scene"))
			{
			}

			if (ImGui::BeginMenu("New Resource"))
			{
				if (ImGui::MenuItem("Material"))
				{
					std::string path = FileDialog::SaveFile("*.material");
					if (!String::EndsWith(path, ".material"))
					{
						path += ".material";
					}

					if (path != "")
					{
						Ref<Material> material = CreateRef<Material>();
						material->IsEmbedded = false;
						auto jsonData = material->Serialize();

						FileSystem::BeginWriteFile(path);
						FileSystem::WriteLine(jsonData.dump(4));
						FileSystem::EndWriteFile();
					    
					    RefreshFileBrowser();
					}
				}

			    if (ImGui::MenuItem("Wren Script"))
			    {
			        std::string path = FileDialog::SaveFile("*.wren");
			        
			        if (!String::EndsWith(path, ".wren"))
			        {
			            path += ".wren";
			        }

			        if (!path.empty())
			        {
                        std::string fileName = String::ToUpper(FileSystem::GetFileNameFromPath(path));
			            fileName = String::RemoveWhiteSpace(fileName);
			            
			            if(!String::IsDigit(fileName[0]))
			            {
			                
                            FileSystem::BeginWriteFile(path);
                            FileSystem::WriteLine(TEMPLATE_SCRIPT_FIRST + fileName + TEMPLATE_SCRIPT_SECOND);
                            FileSystem::EndWriteFile();
			            
                            RefreshFileBrowser();
                        }
			            else
			            {
			                Logger::Log("[FileSystem] Cannot create script files that starts with a number.", CRITICAL);
			            }
			        }
			    }

				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

    }

    void FileSystemUI::RefreshFileBrowser()
    {
        FileSystem::Scan();
        m_CurrentDirectory = FileSystem::RootDirectory;
    }

    float h = 200;
    static float sz1 = 300;
    static float sz2 = 300;
    void FileSystemUI::DrawDirectoryExplorer()
    {
        if (ImGui::Begin("File browser"))
        {
            Ref<Directory> rootDirectory = FileSystem::GetFileTree();
            if (!rootDirectory)
                return;

            ImVec2 avail = ImGui::GetContentRegionAvail();
            Splitter(true, 4.0f, &sz1, &sz2, 100, 8, avail.y);

            ImVec4* colors = ImGui::GetStyle().Colors;
            ImGui::PushStyleColor(ImGuiCol_ChildBg, colors[ImGuiCol_TitleBgCollapsed]);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 8);
            if (ImGui::BeginChild("Tree browser", ImVec2(sz1, avail.y), true))
            {
                ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding;

                bool is_selected = m_CurrentDirectory == FileSystem::RootDirectory;
                if (is_selected)
                    base_flags |= ImGuiTreeNodeFlags_Selected;

                std::string icon = ICON_FA_FOLDER;
                bool open = ImGui::TreeNodeEx((icon + "  Project files").c_str(), base_flags);
                if (ImGui::IsItemClicked())
                {
                    m_CurrentDirectory = FileSystem::RootDirectory;
                }

                if (open)
                {
                    for(auto& d : rootDirectory->Directories)
                        EditorInterfaceDrawFiletree(d);

                    ImGui::TreePop();
                }

            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
            ImGui::EndChild();
            ImGui::SameLine();

            avail = ImGui::GetContentRegionAvail();

            std::vector<Ref<Directory>> paths = std::vector<Ref<Directory>>();

            Ref<Directory> currentParent = m_CurrentDirectory;
            paths.push_back(m_CurrentDirectory);

            while (currentParent != nullptr) 
            {
                paths.push_back(currentParent);
                currentParent = currentParent->Parent;
            }

            avail = ImGui::GetContentRegionAvail();
            if (ImGui::BeginChild("Wrapper", avail))
            {
                avail.y = 30;
                if (ImGui::BeginChild("Path", avail, true))
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

                    const auto buttonSize = ImVec2(24, 24);
                    std::string refreshIcon = ICON_FA_SYNC_ALT;
                    if (ImGui::Button((refreshIcon).c_str(), buttonSize))
                    {
                        RefreshFileBrowser();
                    }

                    ImGui::SameLine();

                    if (ImGui::Button((std::string(ICON_FA_ANGLE_LEFT)).c_str(), buttonSize))
                    {
                        if (m_CurrentDirectory != FileSystem::RootDirectory)
                        {
                            m_CurrentDirectory = m_CurrentDirectory->Parent;
                        }
                    }

                    ImGui::SameLine();

                    if (ImGui::Button((std::string(ICON_FA_ANGLE_RIGHT)).c_str(), buttonSize))
                    {

                    }
                   
                    ImGui::SameLine();

                    ImGui::PushStyleColor(ImGuiCol_ChildBg, colors[ImGuiCol_TitleBgCollapsed]);
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
                    ImGui::BeginChild("pathBrowser", ImVec2(ImGui::GetContentRegionAvailWidth(), 24));
                    for (int i = paths.size() - 1; i > 0; i--) 
                    {
                        if (i != paths.size())
                            ImGui::SameLine();

                        std::string pathLabel;
                        if (i == paths.size() - 1)
                        {
                            pathLabel = "Project files";
                        }
                        else
                        {
                            pathLabel = paths[i]->name;
                        }

                        if (ImGui::Button(pathLabel.c_str()))
                        {
                            m_CurrentDirectory = paths[i];
                        }

                        ImGui::SameLine();
                        ImGui::Text("/");
                    }

                    ImGui::EndChild();
                    ImGui::PopStyleVar();
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleColor();
                }

                ImGui::EndChild();

                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImGui::GetWindowDrawList()->AddLine(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY()), ImVec2(ImGui::GetContentRegionAvailWidth(), ImGui::GetCursorPosY()), IM_COL32(255, 0, 0, 255), 1.0f);

                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
                avail = ImGui::GetContentRegionAvail();

                bool child = ImGui::BeginChild("Content", avail);
                ImGui::PopStyleVar();
                if (child)
                {
                    int width = avail.x;
                    ImVec2 buttonSize = ImVec2(80, 80);
                    int amount = (int)(width / 100);
                    if (amount <= 0) amount = 1;

                    int i = 1; // current amount of item per row.
                    if (ImGui::BeginTable("ssss", amount))
                    {
                        // Button to go up a level.
                        if (m_CurrentDirectory && m_CurrentDirectory != FileSystem::RootDirectory && m_CurrentDirectory->Parent)
                        {
                            ImGui::TableNextColumn();
                            if (ImGui::Button("..", ImVec2(100, 100)))
                                m_CurrentDirectory = m_CurrentDirectory->Parent;
                            i++;
                        }

                        if (m_CurrentDirectory && m_CurrentDirectory->Directories.size() > 0)
                        {
                            for (Ref<Directory>& d : m_CurrentDirectory->Directories)
                            {
                                if (i + 1 % amount != 0)
                                    ImGui::TableNextColumn();
                                else
                                    ImGui::TableNextRow();

                                DrawDirectory(d);
                                i++;
                            }
                        }

                        if (m_CurrentDirectory && m_CurrentDirectory->Files.size() > 0)
                        {
                            for (auto f : m_CurrentDirectory->Files)
                            {
                                if (i - 1 % amount != 0 || i == 1)
                                    ImGui::TableNextColumn();
                                else
                                    ImGui::TableNextRow();
                                
                                DrawFile(f, i);
                                i++;
                            }
                        }

                        DrawContextMenu();
                        m_hasClickedOnFile = false;
                        
                        ImGui::EndTable();
                    }

                }
                ImGui::EndChild();
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }
}

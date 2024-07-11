
#include "FileSystemUI.h"

#include <src/Vendors/imgui/imgui.h>

#include <src/Vendors/imgui/imgui_internal.h>
#include "src/Resource/FontAwesome5.h"
#include "src/Scene/Components/ParentComponent.h"
#include "src/Rendering/Textures/Texture.h"
#include "src/Rendering/Textures/TextureManager.h"
#include "EditorInterface.h"

#include <src/Rendering/Textures/Material.h>
#include "../Misc/PopupHelper.h"

#include "src/Scene/Systems/WadConverter.h"
#include "../Misc/ThumbnailManager.h"

namespace Nuake
{
    Ref<Directory> FileSystemUI::m_CurrentDirectory;
    
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


    std::string renameTempValue = "";

    void FileSystemUI::EditorInterfaceDrawFiletree(Ref<Directory> dir)
    {
        ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_FramePadding;
        //if (is_selected) 
       
        if (m_CurrentDirectory == dir)
            base_flags |= ImGuiTreeNodeFlags_Selected;
        if (dir->Directories.size() <= 0)
            base_flags |= ImGuiTreeNodeFlags_Leaf;

        std::string icon = ICON_FA_FOLDER;
        bool open = ImGui::TreeNodeEx((icon + "  " + dir->Name.c_str()).c_str(), base_flags);

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

    void FileSystemUI::DrawDirectory(Ref<Directory> directory, uint32_t drawId)
    {
        ImGui::PushFont(FontManager::GetFont(Icons));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        const char* icon = ICON_FA_FOLDER;
        const std::string id = std::string("##") + directory->Name;

        ImVec2 prevCursor = ImGui::GetCursorPos();
        ImVec2 prevScreenPos = ImGui::GetCursorScreenPos();
        const bool selected = ImGui::Selectable(id.c_str(), Editor->Selection.Directory == directory, ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_AllowDoubleClick, ImVec2(100, 150));
        const std::string hoverMenuId = std::string("item_hover_menu") + std::to_string(drawId);
        if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(1))
        {
            ImGui::OpenPopup(hoverMenuId.c_str());
            m_HasClickedOnFile = true;
        }

        const std::string renameId = "Rename" + std::string("##") + hoverMenuId;
        bool shouldRename = false;

        const std::string deleteId = "Delete" + std::string("##") + hoverMenuId;
        bool shouldDelete = false;
        
        if (selected)
        {
            if (ImGui::IsMouseDoubleClicked(0))
            {
                m_CurrentDirectory = directory;
            }

            Editor->Selection = EditorSelection(directory);
        }

        if (ImGui::IsItemHovered())
            ImGui::SetTooltip(directory->Name.c_str());


        ImGui::SetCursorPos(prevCursor);
        ImGui::Image((ImTextureID)TextureManager::Get()->GetTexture("Resources/Images/folder_icon.png")->GetID(), ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));

        auto imguiStyle = ImGui::GetStyle();

        ImVec2 startOffset = ImVec2(imguiStyle.CellPadding.x / 2.0f, 0);
        ImVec2 offsetEnd = ImVec2(startOffset.x, imguiStyle.CellPadding.y / 2.0f);
        ImU32 rectColor = IM_COL32(255, 255, 255, 16);
        ImGui::GetWindowDrawList()->AddRectFilled(prevScreenPos + ImVec2(0, 100) - startOffset, prevScreenPos + ImVec2(100, 150) + offsetEnd, rectColor, 1.0f);
        std::string visibleName = directory->Name;
        const uint32_t MAX_CHAR_NAME = 34;
        if (directory->Name.size() > MAX_CHAR_NAME)
        {
            visibleName = std::string(directory->Name.begin(), directory->Name.begin() + MAX_CHAR_NAME - 3) + "...";
        }

        ImGui::TextWrapped(visibleName.c_str());

        ImGui::SetCursorPosY(prevCursor.y + 150 - ImGui::GetTextLineHeight());
        ImGui::TextColored({ 1, 1, 1, 0.5f }, "Folder");

        ImGui::PopStyleVar();
        

        if (ImGui::BeginPopup(hoverMenuId.c_str()))
        {
            if (ImGui::MenuItem("Open"))
            {
                m_CurrentDirectory = directory;
            }

            ImGui::Separator();

            if (ImGui::BeginMenu("Copy"))
            {
                if (ImGui::MenuItem("Full Path"))
                {
                    OS::CopyToClipboard(directory->FullPath);
                }

                if (ImGui::MenuItem("Directory Name"))
                {
                    OS::CopyToClipboard(String::Split(directory->Name, '/')[0]);
                }

                ImGui::EndPopup();
            }

            if (ImGui::MenuItem("Delete"))
            {
                shouldDelete = true;
            }

            if (ImGui::MenuItem("Rename"))
            {
                shouldRename = true;
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Show in File Explorer"))
            {
                OS::OpenIn(directory->FullPath);
            }

            ImGui::EndPopup();
        }

        // Rename Popup
        
        if (shouldRename)
        {
            renameTempValue = directory->Name;
            PopupHelper::OpenPopup(renameId);
        }
        
        if (PopupHelper::DefineTextDialog(renameId, renameTempValue))
        {
            if (OS::RenameDirectory(directory, renameTempValue) != 0)
            {
                Logger::Log("Cannot rename directory: " + renameTempValue, "editor", CRITICAL);
            }
            RefreshFileBrowser();
            renameTempValue = "";
        }

        // Delete Popup

        if(shouldDelete)
        {
            PopupHelper::OpenPopup(deleteId);
        }

        if(PopupHelper::DefineConfirmationDialog(deleteId, " Are you sure you want to delete the folder and all its children?\n This action cannot be undone, and all data within the folder \n will be permanently lost."))
        {
            if (FileSystem::DeleteFolder(directory->FullPath) != 0)
            {
                Logger::Log("Failed to remove directory: " + directory->Name, "editor", CRITICAL);
            }
            RefreshFileBrowser();
        }

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
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, {0.f, 0.f});
        std::string fileExtension = file->GetExtension();

        ImVec2 prevCursor = ImGui::GetCursorPos();
        ImVec2 prevScreenPos = ImGui::GetCursorScreenPos();
        std::string id = std::string("##") + file->GetAbsolutePath();
        const bool selected = ImGui::Selectable(id.c_str(), Editor->Selection.File == file, ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_AllowDoubleClick, ImVec2(100, 150));
       
        const std::string hoverMenuId = std::string("item_hover_menu") + std::to_string(drawId);
        if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(1))
        {
            ImGui::OpenPopup(hoverMenuId.c_str());
            m_HasClickedOnFile = true;
        }

        bool shouldOpenScene = false;
        if (selected)
        {
            if (ImGui::IsMouseDoubleClicked(0))
            { 
                switch (file->GetFileType())
                {
                    case FileType::Map:
                        OS::OpenTrenchbroomMap(file->GetAbsolutePath());
                        break;
                    case FileType::NetScript:
                        OS::OpenIn(file->GetAbsolutePath());
                        break;
                    case FileType::Scene:
                        shouldOpenScene = true;
                        break;
                    case FileType::Solution:
                        OS::OpenIn(file->GetAbsolutePath());
                        break;
                }
                
            }
            
            Editor->Selection = EditorSelection(file);
        }

        if (ImGui::IsItemHovered())
            ImGui::SetTooltip(file->GetName().c_str());

        if (ImGui::BeginDragDropSource())
        {
            char pathBuffer[256];
            std::strncpy(pathBuffer, file->GetAbsolutePath().c_str(), sizeof(pathBuffer));
            std::string dragType;
            if (fileExtension == ".wren")
            {
                dragType = "_Script";
            }
            else if (fileExtension == ".cs")
            {
                dragType = "_NetScript";
            }
            else if (fileExtension == ".map")
            {
                dragType = "_Map";
            }
            else if (fileExtension == ".material")
            {
                dragType = "_Material";
            }
            else if (fileExtension == ".obj" || fileExtension == ".mdl" || fileExtension == ".gltf" || fileExtension == ".md3" || fileExtension == ".fbx" || fileExtension == ".glb")
            {
                dragType = "_Model";
            }
            else if (fileExtension == ".interface")
            {
                dragType = "_Interface";
            }
            else if (fileExtension == ".prefab")
            {
                dragType = "_Prefab";
            }
            else if (fileExtension == ".png" || fileExtension == ".jpg")
            {
                dragType = "_Image";
            }
            else if (fileExtension == ".wav" || fileExtension == ".ogg")
            {
                dragType = "_AudioFile";
            }

            ImGui::SetDragDropPayload(dragType.c_str(), (void*)(pathBuffer), sizeof(pathBuffer));
            ImGui::Text(file->GetName().c_str());
            ImGui::EndDragDropSource();
        }


        Ref<Texture> textureImage =TextureManager::Get()->GetTexture("Resources/Images/file_icon.png");

        const auto textureMgr = TextureManager::Get();
        const auto fileType = file->GetFileType();
        if (fileType == FileType::Material)
        {
            textureImage = ThumbnailManager::Get().GetThumbnail(file->GetRelativePath());
        }
        else if (fileType == FileType::Image)
        {
            const std::string path = file->GetAbsolutePath();
            textureImage = textureMgr->GetTexture(path);
        }
        else if (fileType == FileType::Project)
        {
            textureImage = textureMgr->GetTexture("Resources/Images/project_icon.png");
        }
        else if (fileType == FileType::NetScript)
        {
            textureImage = textureMgr->GetTexture("Resources/Images/csharp_icon.png");
        }
        else if (fileType == FileType::Scene)
        {
            textureImage = textureMgr->GetTexture("Resources/Images/scene_icon.png");
        }
        else if (fileType == FileType::Script)
        {
            textureImage = textureMgr->GetTexture("Resources/Images/script_file_icon.png");
        }
        else if (fileType == FileType::Audio)
        {
            textureImage = textureMgr->GetTexture("Resources/Images/Audio_file_icon.png");
        }
        else if (fileType == FileType::Prefab)
        {
            textureImage = ThumbnailManager::Get().GetThumbnail(file->GetRelativePath());
        }

        ImGui::SetCursorPos(prevCursor);
        ImGui::Image(reinterpret_cast<ImTextureID>(textureImage->GetID()), ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::PopStyleVar();
        
        auto imguiStyle = ImGui::GetStyle();

        ImVec2 startOffset = ImVec2(imguiStyle.CellPadding.x / 2.0f, 0);
        ImVec2 offsetEnd = ImVec2(startOffset.x, imguiStyle.CellPadding.y / 2.0f);
        ImU32 rectColor = IM_COL32(255, 255, 255, 16);
        ImGui::GetWindowDrawList()->AddRectFilled(prevScreenPos + ImVec2(0, 100) - startOffset, prevScreenPos + ImVec2(100, 150) + offsetEnd, rectColor, 1.0f);
        std::string visibleName = file->GetName();
        const uint32_t MAX_CHAR_NAME = 32;
        if (file->GetName().size() >= MAX_CHAR_NAME)
        {
            visibleName = std::string(visibleName.begin(), visibleName.begin() + MAX_CHAR_NAME - 3) + "...";
        }

        ImGui::TextWrapped(visibleName.c_str());

        ImGui::SetCursorPosY(prevCursor.y + 150 - ImGui::GetTextLineHeight());
        ImGui::TextColored({ 1, 1, 1, 0.5f }, file->GetFileTypeAsString().c_str());
        
        //if (fileExtension == ".png" || fileExtension == ".jpg")
        //{
        //    
        //}
        //else
        //{
        //    const char* icon = ICON_FA_FILE;
        //    if (fileExtension == ".shader" || fileExtension == ".wren")
        //        icon = ICON_FA_FILE_CODE;
        //    if (fileExtension == ".map")
        //        icon = ICON_FA_BROOM;
        //    if (fileExtension == ".ogg" || fileExtension == ".mp3" || fileExtension == ".wav")
        //        icon = ICON_FA_FILE_AUDIO;
        //    if (fileExtension == ".gltf" || fileExtension == ".obj")
        //        icon = ICON_FA_FILE_IMAGE;
        //
        //    std::string fullName = icon + std::string("##") + file->GetAbsolutePath();
        //
        //    bool pressed = false;
        //    if (fileExtension == ".material")
        //    {
        //        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        //        pressed = ImGui::ImageButton(fullName.c_str(), (void*)ThumbnailManager::Get().GetThumbnail(file-//>GetRelativePath())->GetID(), ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
        //        ImGui::PopStyleVar();
        //    }
        //    else
        //    {
        //        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        //        pressed = ImGui::Button(fullName.c_str(), ImVec2(100, 100));
        //        ImGui::PopStyleVar();
        //    }
        //   
        if (Editor->Selection.File == file && Editor->Selection.File->GetFileType() != FileType::Prefab)
        {
            ThumbnailManager::Get().MarkThumbnailAsDirty(file->GetRelativePath());
        }

        //    if(pressed)
        //    {
        //        Editor->Selection = EditorSelection(file);
        //    }
        //
        //    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
        //    {
        //        OS::OpenTrenchbroomMap(file->GetAbsolutePath());
        //    }
        //}
        ImGui::PopStyleVar();

        const std::string openSceneId = "Open Scene" + std::string("##") + hoverMenuId;
        

        const std::string renameId = "Rename" + std::string("##") + hoverMenuId;
        bool shouldRename = false;

        const std::string deleteId = "Delete" + std::string("##") + hoverMenuId;
        bool shouldDelete = false;

        if (ImGui::BeginPopup(hoverMenuId.c_str()))
        {
            if (file->GetExtension() != ".scene") 
            {
                if (ImGui::MenuItem("Open in Editor"))
                {
                    OS::OpenIn(file->GetAbsolutePath());
                }
            }
            else
            {
                if (ImGui::MenuItem("Load Scene"))
                {
                    shouldOpenScene = true;
                }
            }

            ImGui::Separator();

            if (ImGui::BeginMenu("Copy"))
            {
                if (ImGui::MenuItem("Full Path"))
                {
                    OS::CopyToClipboard(file->GetAbsolutePath());
                }

                if (ImGui::MenuItem("File Name"))
                {
                    OS::CopyToClipboard(file->GetName());
                }

                ImGui::EndPopup();
            }

            if (file->GetExtension() != ".project")
            {
                if (ImGui::MenuItem("Delete"))
                {
                    shouldDelete = true;
                }
            }
            else 
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 0.2f));
                ImGui::MenuItem("Delete");
                ImGui::PopStyleColor();
                if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();
                    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                    ImGui::TextUnformatted("The file you're trying to delete is currently loaded by the game engine.");
                    ImGui::PopTextWrapPos();
                    ImGui::EndTooltip();
                }
            }
            
            if (file->GetExtension() == ".wad")
            {
                if (ImGui::MenuItem("Convert to Materials"))
                {
                    Nuake::ExtractWad(file->GetAbsolutePath(), FileSystem::Root);
                }
            }

            if (ImGui::MenuItem("Rename"))
            {
                shouldRename = true;
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Show in File Explorer"))
            {
                OS::ShowInFileExplorer(file->GetAbsolutePath());
            }
            
            ImGui::EndPopup();
        }

        // Open Scene Popup

        if (shouldOpenScene)
        {
            PopupHelper::OpenPopup(openSceneId);
        }

        if (PopupHelper::DefineConfirmationDialog(openSceneId, " Open the scene? \n Changes will not be saved."))
        {
            Ref<Scene> scene = Scene::New();
            const std::string projectPath = file->GetAbsolutePath();
            if (!scene->Deserialize(FileSystem::ReadFile(projectPath, true)))
            {
                Logger::Log("Failed loading scene: " + projectPath,"editor", CRITICAL);
                return;
            }

            FileSystem::SetRootDirectory(projectPath + "/");
            scene->Path = FileSystem::AbsoluteToRelative(projectPath);
            Engine::LoadScene(scene);
        }

        // Rename Popup

        if (shouldRename)
        {
            renameTempValue = file->GetName();
            PopupHelper::OpenPopup(renameId);
        }

        if (PopupHelper::DefineTextDialog(renameId, renameTempValue))
        {
            if(OS::RenameFile(file, renameTempValue) != 0)
            {
                Logger::Log("Cannot rename file: " + renameTempValue, "editor", CRITICAL);
            }
            RefreshFileBrowser();
            renameTempValue = "";
        }

        // Delete Popup

        if(shouldDelete)
        {
            PopupHelper::OpenPopup(deleteId);
        }

        if(PopupHelper::DefineConfirmationDialog(deleteId, " Are you sure you want to delete the file?\n This action cannot be undone, and all data \n will be permanently lost."))
        {
            if (FileSystem::DeleteFileFromPath(file->GetAbsolutePath()) != 0)
            {
                Logger::Log("Failed to remove file: " + file->GetRelativePath(), "editor", CRITICAL);
            }
            RefreshFileBrowser();
        }

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
        if (!m_HasClickedOnFile && ImGui::IsMouseReleased(1) && ImGui::IsWindowHovered())
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
					const std::string path = FileDialog::SaveFile("*.material");
					if (!path.empty())
					{
                        std::string finalPath = path;
                        if (!String::EndsWith(path, ".material"))
                        {
                            finalPath = path + ".material";
                        }

						Ref<Material> material = CreateRef<Material>();
						material->IsEmbedded = false;
						auto jsonData = material->Serialize();

						FileSystem::BeginWriteFile(finalPath, true);
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
			                
                            FileSystem::BeginWriteFile(path, true);
                            FileSystem::WriteLine(TEMPLATE_SCRIPT_FIRST + fileName + TEMPLATE_SCRIPT_SECOND);
                            FileSystem::EndWriteFile();
			            
                            RefreshFileBrowser();
                        }
			            else
			            {
			                Logger::Log("Cannot create script files that starts with a number.", "filesystem", CRITICAL);
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
        //if (ImGui::Begin("thumbnail debugger"))
        //{
        //    auto texture = ThumbnailManager::Get().GetThumbnail("Materials/prototype_1_2/64_blood_2.material");
        //    ImGui::Image((void*)texture->GetID(), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
        //}
        //ImGui::End();

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
                ImGui::PushFont(FontManager::GetFont(Bold));
                bool open = ImGui::TreeNodeEx("PROJECT", base_flags);
                if (ImGui::IsItemClicked())
                {
                    m_CurrentDirectory = FileSystem::RootDirectory;
                }
                ImGui::PopFont();

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
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 2, 4 });
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

                    const auto buttonSize = ImVec2(26, 26);
                    std::string refreshIcon = ICON_FA_SYNC_ALT;
                    if (ImGui::Button((refreshIcon).c_str(), buttonSize))
                    {
                        RefreshFileBrowser();
                    }

                    ImGui::SameLine();

                    const auto cursorStart = ImGui::GetCursorPosX();
                    if (ImGui::Button((std::string(ICON_FA_ANGLE_LEFT)).c_str(), buttonSize))
                    {
                        if (m_CurrentDirectory != FileSystem::RootDirectory)
                        {
                            m_CurrentDirectory = m_CurrentDirectory->Parent;
                        }
                    }

                    ImGui::SameLine();

                    const auto cursorEnd = ImGui::GetCursorPosX();
                    const auto buttonWidth = cursorEnd - cursorStart;
                    if (ImGui::Button((std::string(ICON_FA_ANGLE_RIGHT)).c_str(), buttonSize))
                    {
                        if (Editor->Selection.Type == EditorSelectionType::Directory)
                        {
                            m_CurrentDirectory = Editor->Selection.Directory;
                        }
                    }
                   
                    const uint32_t numButtonAfterPathBrowser = 2;
                    const uint32_t searchBarSize = 6; 
                    ImGui::SameLine();

                    ImGui::PushStyleColor(ImGuiCol_ChildBg, colors[ImGuiCol_TitleBgCollapsed]);
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
                    ImGui::BeginChild("pathBrowser", ImVec2((ImGui::GetContentRegionAvail().x - (numButtonAfterPathBrowser * buttonWidth * searchBarSize)) - 4.0, 24));
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
                            pathLabel = paths[i]->Name;
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
                    
                    ImGui::SameLine();

                    ImGui::BeginChild("searchBar", ImVec2(ImGui::GetContentRegionAvail().x - (numButtonAfterPathBrowser * buttonWidth), 24));
                    char buffer[256];
                    memset(buffer, 0, sizeof(buffer));
                    std::strncpy(buffer, m_SearchKeyword.c_str(), sizeof(buffer));
                    if (ImGui::InputTextEx("##Search", "Asset search & filter ..", buffer, sizeof(buffer), ImVec2(ImGui::GetContentRegionAvail().x, 24), ImGuiInputTextFlags_EscapeClearsAll))
                    {
                        m_SearchKeyword = std::string(buffer);
                    }
                    ImGui::EndChild();

                    ImGui::SameLine();

                    if (ImGui::Button((std::string(ICON_FA_FOLDER_OPEN)).c_str(), buttonSize))
                    {
                        OS::OpenIn(m_CurrentDirectory->FullPath);
                    }

                    ImGui::PopStyleColor(); // Button color
                    
                    ImGui::SameLine();
                    ImGui::PopStyleVar();
                }
                ImGui::EndChild();

                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImGui::GetWindowDrawList()->AddLine(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY()), ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetCursorPosY()), IM_COL32(255, 0, 0, 255), 1.0f);

                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
                avail = ImGui::GetContentRegionAvail();

                bool child = ImGui::BeginChild("Content", avail);
                ImGui::PopStyleVar();
                ImGui::SameLine();
                if (child)
                {
                    int width = avail.x;
                    ImVec2 buttonSize = ImVec2(80, 80);
                    int amount = (int)(width / 110);
                    if (amount <= 0) amount = 1;

                    int i = 1; // current amount of item per row.
                    if (ImGui::BeginTable("ssss", amount))
                    {
                        // Button to go up a level.
                        //if (m_CurrentDirectory && m_CurrentDirectory != FileSystem::RootDirectory && m_CurrentDirectory->Parent)
                        //{
                        //    ImGui::TableNextColumn();
                        //    if (ImGui::Button("..", buttonSize))
                        //        m_CurrentDirectory = m_CurrentDirectory->Parent;
                        //    i++;
                        //}

                        if (m_CurrentDirectory && m_CurrentDirectory->Directories.size() > 0)
                        {
                            for (Ref<Directory>& d : m_CurrentDirectory->Directories)
                            {
                                if(String::Sanitize(d->Name).find(String::Sanitize(m_SearchKeyword)) != std::string::npos)
                                {
                                    if (i + 1 % amount != 0)
                                        ImGui::TableNextColumn();
                                    else
                                        ImGui::TableNextRow();
                        
                                    DrawDirectory(d, i);
                                    i++;
                                }
                            }
                        }

                        if (m_CurrentDirectory && m_CurrentDirectory->Files.size() > 0)
                        {
                            for (auto& f : m_CurrentDirectory->Files)
                            {
                                if(m_SearchKeyword.empty() || f->GetName().find(String::Sanitize(m_SearchKeyword)) != std::string::npos)
                                {
                                    if (i + 1 % amount != 0 || i == 1)
                                    {
                                        ImGui::TableNextColumn();
                                    }
                                    else
                                    {
                                        ImGui::TableNextRow();
                                    }
                                
                                    DrawFile(f, i);
                                    i++;
                                }
                            }
                        }

                        DrawContextMenu();
                        m_HasClickedOnFile = false;
                        
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

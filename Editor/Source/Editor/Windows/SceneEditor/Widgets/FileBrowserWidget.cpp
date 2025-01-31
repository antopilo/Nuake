#include "FileBrowserWidget.h"

#include "../../../Misc/PopupHelper.h"
#include "../../../Events/EditorRequests.h"

#include <Nuake/FileSystem/Directory.h>
#include <Nuake/FileSystem/File.h>
#include <Nuake/Resource/Project.h>
#include <Nuake/Core/String.h>

#include <Nuake/UI/ImUI.h>
#include "../../../Misc/InterfaceFonts.h"

using namespace Nuake;

FileBrowserWidget::FileBrowserWidget(EditorContext& inCtx) : IEditorWidget(inCtx)
{
	
}

void FileBrowserWidget::Update(float ts)
{

}

void FileBrowserWidget::Draw()
{
	if (BeginWidgetWindow("File Browser"))
	{
		Ref<Nuake::Directory> rootDirectory = FileSystem::GetFileTree();

		auto availableSpace = ImGui::GetContentRegionAvail();
		UI::Splitter(true, 4.0f, &splitterSizeLeft, &splitterSizeRight, 100, 8, availableSpace.y);

		ImVec4* colors = ImGui::GetStyle().Colors;
		ImGui::PushStyleColor(ImGuiCol_ChildBg, colors[ImGuiCol_TitleBgCollapsed]);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 8);

		if (ImGui::BeginChild("Tree", ImVec2(splitterSizeLeft, availableSpace.y), true))
		{
			ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanAvailWidth | 
											ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen | 
											ImGuiTreeNodeFlags_FramePadding;

			bool isSelected = this->currentDirectory == FileSystem::RootDirectory;
			if (isSelected)
			{
				base_flags |= ImGuiTreeNodeFlags_Selected;
			}

			// Header
			{
				UIFont boldFont = UIFont(Fonts::Bold);
				bool open = ImGui::TreeNodeEx("PROJECT", base_flags);
				if (ImGui::IsItemClicked())
				{
					this->currentDirectory = FileSystem::RootDirectory;
				}
			}
			
			// Draw tree
			for (auto& d : rootDirectory->Directories)
			{
				DrawFiletree(d);
			}

			ImGui::TreePop();
		}
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
		ImGui::EndChild();

		ImGui::SameLine();

		// Build file path buttons
		auto paths = std::vector<Ref<Nuake::Directory>>();
		{
			Ref<Nuake::Directory> currentParent = currentDirectory;
			paths.push_back(currentDirectory);

			// Recursively build the path to the root
			while (currentParent != nullptr)
			{
				paths.push_back(currentParent);
				currentParent = currentParent->Parent;
			}
		}
		
		availableSpace = ImGui::GetContentRegionAvail();
		if (ImGui::BeginChild("Wrapper", availableSpace))
		{
			availableSpace.y = 30;

			if (ImGui::BeginChild("Path", availableSpace, true))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 2, 4 });
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

				const auto buttonSize = ImVec2(26, 26);

				// Refresh button
				std::string refreshIcon = ICON_FA_SYNC_ALT;
				if (ImGui::Button((refreshIcon).c_str(), buttonSize))
				{
					// RefreshFileBrowser();
				}

				ImGui::SameLine();

				const auto cursorStart = ImGui::GetCursorPosX();
				{	// Go back
					if (ImGui::Button((std::string(ICON_FA_ANGLE_LEFT)).c_str(), buttonSize))
					{
						if (currentDirectory != FileSystem::RootDirectory)
						{
							currentDirectory = currentDirectory->Parent;
						}
					}
				}

                ImGui::SameLine();

				const auto cursorEnd = ImGui::GetCursorPosX();
				const auto buttonWidth = cursorEnd - cursorStart;

				if (ImGui::Button((std::string(ICON_FA_ANGLE_RIGHT)).c_str(), buttonSize))
				{
					if (editorContext.GetSelection().Type == EditorSelectionType::Directory)
					{
						currentDirectory = editorContext.GetSelection().Directory;
					}
				}

				const uint32_t numButtonAfterPathBrowser = 2;
				const uint32_t searchBarSize = 6;
				ImGui::SameLine();

				// Draw path buttons
				{
					ImGui::PushStyleColor(ImGuiCol_ChildBg, colors[ImGuiCol_TitleBgCollapsed]);
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
					ImGui::BeginChild("pathBrowser", ImVec2((ImGui::GetContentRegionAvail().x - (numButtonAfterPathBrowser * buttonWidth * searchBarSize)) - 4.0, 24));
					for (int i = paths.size() - 1; i > 0; i--)
					{
						if (i != paths.size())
						{
							ImGui::SameLine();
						}

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
							currentDirectory = paths[i];
						}

						ImGui::SameLine();
						ImGui::Text("/");
					}
					ImGui::EndChild();
					ImGui::PopStyleVar();
					ImGui::PopStyleVar();
					ImGui::PopStyleColor();
				}

				ImGui::SameLine();

				// Search bar
				ImGui::BeginChild("searchBar", ImVec2(ImGui::GetContentRegionAvail().x - (numButtonAfterPathBrowser * buttonWidth), 24));
				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				std::strncpy(buffer, searchQuery.c_str(), sizeof(buffer));
				if (ImGui::InputTextEx("##Search", "Asset search & filter ..", buffer, sizeof(buffer), ImVec2(ImGui::GetContentRegionAvail().x, 24), ImGuiInputTextFlags_EscapeClearsAll))
				{
					searchQuery = std::string(buffer);
				}
				ImGui::EndChild();

				ImGui::SameLine();

				if (ImGui::Button((std::string(ICON_FA_FOLDER_OPEN)).c_str(), buttonSize))
				{
					OS::OpenIn(currentDirectory->FullPath);
				}
				ImGui::PopStyleColor(); // Button color

				ImGui::SameLine();
				ImGui::PopStyleVar();
			}
			ImGui::EndChild();

			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImGui::GetWindowDrawList()->AddLine(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY()), ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetCursorPosY()), IM_COL32(255, 0, 0, 255), 1.0f);

			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
			availableSpace = ImGui::GetContentRegionAvail();

			bool child = ImGui::BeginChild("Content", availableSpace);
			ImGui::PopStyleVar();
			ImGui::SameLine();
			if (child)
			{
				int width = availableSpace.x;
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

					if (currentDirectory && currentDirectory->Directories.size() > 0)
					{
						for (Ref<Nuake::Directory>& d : currentDirectory->Directories)
						{
							if (d->GetName() == "bin" || d->GetName() == ".vs" || d->GetName() == "obj")
							{
								continue;
							}

							if (Nuake::String::Sanitize(d->Name).find(Nuake::String::Sanitize(searchQuery)) != std::string::npos)
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

					if (currentDirectory && currentDirectory->Files.size() > 0)
					{
						for (auto& f : currentDirectory->Files)
						{
							if (searchQuery.empty() || f->GetName().find(String::Sanitize(searchQuery)) != std::string::npos)
							{
								if (f->GetFileType() == FileType::Unknown || f->GetFileType() == FileType::Assembly)
								{
									continue;
								}

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

					//DrawContextMenu();

					//m_HasClickedOnFile = false;

					ImGui::EndTable();
				}
			}
			ImGui::EndChild();
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void FileBrowserWidget::DrawFiletree(Ref<Nuake::Directory> dir)
{
	ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | 
									ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_FramePadding;

	if (currentDirectory == dir)
	{
		base_flags |= ImGuiTreeNodeFlags_Selected;
	}

	if (dir->Directories.size() <= 0)
	{
		base_flags |= ImGuiTreeNodeFlags_Leaf;
	}

	std::string icon = ICON_FA_FOLDER;
	bool open = ImGui::TreeNodeEx((icon + "  " + dir->Name.c_str()).c_str(), base_flags);

	if (ImGui::IsItemClicked())
	{
		currentDirectory = dir;
	}

	if (open)
	{
		for (auto& d : dir->Directories)
		{
			DrawFiletree(d);
		}

		ImGui::TreePop();
	}
}

void FileBrowserWidget::DrawDirectory(Ref<Nuake::Directory> directory, uint32_t drawId)
{
	ImGui::PushFont(FontManager::GetFont(Icons));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
	const char* icon = ICON_FA_FOLDER;
	const std::string id = std::string("##") + directory->Name;

	ImVec2 prevCursor = ImGui::GetCursorPos();
	ImVec2 prevScreenPos = ImGui::GetCursorScreenPos();
	const bool selected = ImGui::Selectable(id.c_str(), editorContext.GetSelection().Directory == directory, ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_AllowDoubleClick, ImVec2(100, 150));
	const std::string hoverMenuId = std::string("item_hover_menu") + std::to_string(drawId);
	if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(1))
	{
		ImGui::OpenPopup(hoverMenuId.c_str());
		//m_HasClickedOnFile = true;
	}

	const std::string renameId = "Rename" + std::string("##") + hoverMenuId;
	bool shouldRename = false;

	const std::string deleteId = "Delete" + std::string("##") + hoverMenuId;
	bool shouldDelete = false;

	if (selected)
	{
		if (ImGui::IsMouseDoubleClicked(0))
		{
			currentDirectory = directory;
		}

		//Editor->Selection = EditorSelection(directory);
	}

	if (ImGui::IsItemHovered())
		ImGui::SetTooltip(directory->Name.c_str());


	ImGui::SetCursorPos(prevCursor);
	ImGui::Image((ImTextureID)TextureManager::Get()->GetTexture2("Resources/Images/folder_icon.png")->GetImGuiDescriptorSet(), ImVec2(100, 100));

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
			currentDirectory = directory;
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
		//renameTempValue = directory->Name;
		//PopupHelper::OpenPopup(renameId);
	}

	//if (PopupHelper::DefineTextDialog(renameId, renameTempValue))
	//{
//		if (OS::RenameDirectory(directory, renameTempValue) != 0)
//		{
//			Logger::Log("Cannot rename directory: " + renameTempValue, "editor", CRITICAL);
//		}
//		//RefreshFileBrowser();
//		renameTempValue = "";
	//}

	// Delete Popup

	if (shouldDelete)
	{
		PopupHelper::OpenPopup(deleteId);
	}

	if (PopupHelper::DefineConfirmationDialog(deleteId, " Are you sure you want to delete the folder and all its children?\n This action cannot be undone, and all data within the folder \n will be permanently lost."))
	{
		if (FileSystem::DeleteFolder(directory->FullPath) != 0)
		{
			Logger::Log("Failed to remove directory: " + directory->Name, "editor", CRITICAL);
		}
		//RefreshFileBrowser();
	}

	ImGui::PopFont();
}

void FileBrowserWidget::DrawFile(Ref<Nuake::File> file, uint32_t drawId)
{
    //ImGui::PushFont(EditorInterface::bigIconFont);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 0.f, 0.f });
    std::string fileExtension = file->GetExtension();

    ImVec2 prevCursor = ImGui::GetCursorPos();
    ImVec2 prevScreenPos = ImGui::GetCursorScreenPos();
    std::string id = std::string("##") + file->GetAbsolutePath();
    const bool selected = ImGui::Selectable(id.c_str(), editorContext.GetSelection().File == file, ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_AllowDoubleClick, ImVec2(100, 150));

    const std::string hoverMenuId = std::string("item_hover_menu") + std::to_string(drawId);
    if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(1))
    {
        ImGui::OpenPopup(hoverMenuId.c_str());
        //m_HasClickedOnFile = true;
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
            case FileType::UI:
            case FileType::CSS:
                OS::OpenIn(file->GetAbsolutePath());
                break;
            case FileType::Scene:
                EditorRequests::Get().RequestLoadScene(file);
                break;
            case FileType::Solution:
                OS::OpenIn(file->GetAbsolutePath());
                break;
            case FileType::Prefab:
                //this->Editor->OpenPrefabWindow(file->GetRelativePath());
                break;
            }
        }

		editorContext.SetSelection(EditorSelection(file));
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
        else if (fileExtension == ".nkmesh" || fileExtension == ".obj" || fileExtension == ".mdl" || fileExtension == ".gltf" || fileExtension == ".md3" || fileExtension == ".fbx" || fileExtension == ".glb")
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
        else if (fileExtension == ".html")
        {
            dragType = "_UIFile";
        }
        else if (fileExtension == ".sky")
        {
            dragType = "_SkyFile";
        }
        else if (fileExtension == ".env")
        {
            dragType = "_EnvFile";
        }

        ImGui::SetDragDropPayload(dragType.c_str(), (void*)(pathBuffer), sizeof(pathBuffer));
        ImGui::Text(file->GetName().c_str());
        ImGui::EndDragDropSource();
    }


    Ref<VulkanImage> textureImage = TextureManager::Get()->GetTexture2("Resources/Images/file_icon.png");

    const auto textureMgr = TextureManager::Get();
    const auto fileType = file->GetFileType();
    if (fileType == FileType::Material)
    {
        //auto image = ThumbnailManager::Get().GetThumbnail(file->GetRelativePath());
        //if (image)
        //{
        //    textureImage = image;
        //}
    }
    else if (fileType == FileType::Image)
    {
        const std::string path = file->GetAbsolutePath();
        textureImage = textureMgr->GetTexture2(path);
    }
    else if (fileType == FileType::Project)
    {
        textureImage = textureMgr->GetTexture2("Resources/Images/project_icon.png");
    }
    else if (fileType == FileType::NetScript)
    {
        textureImage = textureMgr->GetTexture2("Resources/Images/csharp_icon.png");
    }
    else if (fileType == FileType::Scene)
    {
        textureImage = textureMgr->GetTexture2("Resources/Images/scene_icon.png");
    }
    else if (fileType == FileType::Script)
    {
        textureImage = textureMgr->GetTexture2("Resources/Images/script_file_icon.png");
    }
    else if (fileType == FileType::Audio)
    {
        textureImage = textureMgr->GetTexture2("Resources/Images/audio_file_icon.png");
    }
    else if (fileType == FileType::Prefab)
    {
        //auto image = ThumbnailManager::Get().GetThumbnail(file->GetRelativePath());
        //if (image)
        //{
        //    textureImage = image;
        //}
    }
    else if (fileType == FileType::Mesh)
    {
        //auto image = ThumbnailManager::Get().GetThumbnail(file->GetRelativePath());
        //if (image)
        //{
        //    textureImage = image;
        //}
    }
    else if (fileType == FileType::Solution)
    {
        textureImage = textureMgr->GetTexture2("Resources/Images/sln_icon.png");
    }
    else if (fileType == FileType::Map)
    {
        textureImage = textureMgr->GetTexture2("Resources/Images/trenchbroom_icon.png");
    }
    else if (fileType == FileType::Env)
    {
        textureImage = textureMgr->GetTexture2("Resources/Images/env_file_icon.png");
    }

    ImGui::SetCursorPos(prevCursor);
    ImGui::Image(reinterpret_cast<ImTextureID>(textureImage->GetImGuiDescriptorSet()), ImVec2(100, 100));
    ImGui::PopStyleVar();

    auto& imguiStyle = ImGui::GetStyle();

    ImVec2 startOffset = ImVec2(imguiStyle.CellPadding.x / 2.0f, 0);
    ImVec2 offsetEnd = ImVec2(startOffset.x, imguiStyle.CellPadding.y / 2.0f);
    ImU32 rectColor = IM_COL32(255, 255, 255, 16);
    ImGui::GetWindowDrawList()->AddRectFilled(prevScreenPos + ImVec2(0, 100) - startOffset, prevScreenPos + ImVec2(100, 150) + offsetEnd, rectColor, 1.0f);

    ImU32 rectColor2 = UI::PrimaryCol;
    Color fileTypeColor = GetColorByFileType(file->GetFileType());
    ImGui::GetWindowDrawList()->AddRectFilled(prevScreenPos + ImVec2(0, 100) - startOffset, prevScreenPos + ImVec2(100, 101) + offsetEnd, IM_COL32(fileTypeColor.r * 255.f, fileTypeColor.g * 255.f, fileTypeColor.b * 255.f, fileTypeColor.a * 255.f), 0.0f);

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
    if (editorContext.GetSelection().File == file && editorContext.GetSelection().File->GetFileType() != FileType::Prefab)
    {
        //ThumbnailManager::Get().MarkThumbnailAsDirty(file->GetRelativePath());
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
                //Nuake::ExtractWad(file->GetAbsolutePath(), FileSystem::Root);
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
        if (!scene->Deserialize(json::parse(FileSystem::ReadFile(projectPath, true))))
        {
            Logger::Log("Failed loading scene: " + projectPath, "editor", CRITICAL);
            ImGui::PopFont();
            return;
        }

        scene->Path = FileSystem::AbsoluteToRelative(projectPath);
        Engine::SetCurrentScene(scene);
    }

    // Rename Popup

    if (shouldRename)
    {
        //renameTempValue = file->GetName();
        PopupHelper::OpenPopup(renameId);
    }

    //if (PopupHelper::DefineTextDialog(renameId, renameTempValue))
    //{
    //    if (OS::RenameFile(file, renameTempValue) != 0)
    //    {
    //        Logger::Log("Cannot rename file: " + renameTempValue, "editor", CRITICAL);
    //    }
    //    RefreshFileBrowser();
    //    renameTempValue = "";
    //}

    // Delete Popup

    if (shouldDelete)
    {
        PopupHelper::OpenPopup(deleteId);
    }

    if (PopupHelper::DefineConfirmationDialog(deleteId, " Are you sure you want to delete the file?\n This action cannot be undone, and all data \n will be permanently lost."))
    {
        if (FileSystem::DeleteFileFromPath(file->GetAbsolutePath()) != 0)
        {
            Logger::Log("Failed to remove file: " + file->GetRelativePath(), "editor", CRITICAL);
        }
        //RefreshFileBrowser();
    }

    //ImGui::PopFont();
}

Color FileBrowserWidget::GetColorByFileType(Nuake::FileType fileType)
{
    {
        switch (fileType)
        {
        case Nuake::FileType::Unknown:
            break;
        case Nuake::FileType::Image:
            break;
        case Nuake::FileType::Material:
            break;
        case Nuake::FileType::Mesh:
            break;
        case Nuake::FileType::Script:
            return { 1.0, 0.0, 0.0, 1.0 };
            break;
        case Nuake::FileType::NetScript:
            return { 1.0, 0.0, 0.0, 1.0 };
            break;
        case Nuake::FileType::Project:
            return Engine::GetProject()->Settings.PrimaryColor;
            break;
        case Nuake::FileType::Prefab:
            break;
        case Nuake::FileType::Scene:
            return { 0, 1.0f, 1.0, 1.0 };
            break;
        case Nuake::FileType::Wad:
            break;
        case Nuake::FileType::Map:
            return { 0.0, 1.0, 0.0, 1.0 };
            break;
        case Nuake::FileType::Assembly:
            break;
        case Nuake::FileType::Solution:
            break;
        case Nuake::FileType::Audio:
            return { 0.0, 0.0, 1.0, 1.0 };
            break;
        case Nuake::FileType::UI:
            return { 1.0, 1.0, 0.0, 1.0 };
            break;
        case Nuake::FileType::CSS:
            return { 1.0, 0.0, 1.0, 1.0 };
            break;
        default:
            break;
        }

        return Color(0, 0, 0, 0);
    }
}

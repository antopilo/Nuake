#include "FileSystemUI.h"

#include <src/Vendors/imgui/imgui.h>

#include <src/Vendors/imgui/imgui_internal.h>
#include "src/Resource/FontAwesome5.h"
#include "src/Scene/Components/ParentComponent.h"
#include "src/Rendering/Textures/Texture.h"
#include "src/Rendering/Textures/TextureManager.h"
#include "EditorInterface.h"

const std::string TEMPLATE_SCRIPT_BEGIN = "import \"Nuake:Engine\" for Engine \
import \"Nuake:ScriptableEntity\" for ScriptableEntity \
import \"Nuake:Input\" for Input \
import \"Nuake:Scene\" for Scene \
\
class ";

const std::string TEMPLATE_SCRIPT_END = " is ScriptableEntity {\
construct new(){\
	_ReloadSpeed = 0.1\
	_Intensity = 0.0\
}\
\
init() {\
}\
\
// Updates every frame\
update(ts) {\
\
}\
\
// Updates every tick\
fixedUpdate(ts) {\
	\
}\
\
exit() {\
}\
}";

#include <src/Rendering/Textures/Material.h>

namespace Nuake {
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
        ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
        //if (is_selected) 
       
        std::string icon = ICON_FA_FOLDER;
        if (m_CurrentDirectory == dir)
            base_flags |= ImGuiTreeNodeFlags_Selected;
        if (dir->Directories.size() <= 0)
            base_flags = ImGuiTreeNodeFlags_Leaf;

        bool open = ImGui::TreeNodeEx(dir->name.c_str(), base_flags);

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
        ImGui::PushFont(EditorInterface::bigIconFont);
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

    void FileSystemUI::DrawFile(Ref<File> file)
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
		if (ImGui::BeginPopupContextWindow())
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
					}
				}

				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

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

            if (ImGui::BeginChild("Tree browser", ImVec2(sz1, avail.y)))
            {
                ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

                bool is_selected = m_CurrentDirectory == FileSystem::RootDirectory;
                if (is_selected)
                    base_flags |= ImGuiTreeNodeFlags_Selected;

                std::string icon = ICON_FA_FOLDER;
                bool open = ImGui::TreeNodeEx((icon + " " + "Project files").c_str(), base_flags);
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
                if (ImGui::BeginChild("Path", avail))
                {
                    if (ImGui::Button("Refresh"))
                    {
                        FileSystem::Scan();
                        m_CurrentDirectory = FileSystem::RootDirectory;
                    }

                    ImGui::SameLine();
                    for (int i = paths.size() - 1; i > 0; i--) 
                    {
                        if (i != paths.size())
                            ImGui::SameLine();

                        if (ImGui::Button(paths[i]->name.c_str()))
                            m_CurrentDirectory = paths[i];

                        ImGui::SameLine();
                        ImGui::Text("/");
                    }
                }
                ImGui::EndChild();

                avail = ImGui::GetContentRegionAvail();

                if (ImGui::BeginChild("Content", avail))
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
                            ImGui::TableNextColumn();
                            i++;
                        }

                        if (m_CurrentDirectory && m_CurrentDirectory->Directories.size() > 0)
                        {
                            for (Ref<Directory>& d : m_CurrentDirectory->Directories)
                            {
                                DrawDirectory(d);

                                if (i + 1 % amount != 0)
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

                        DrawContextMenu();
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

#include <map>
#include <algorithm>

#include "ProjectInterface.h"
#include "EditorInterface.h"

#include "Engine.h"
#include "src/Core/Logger.h"

#include "src/Vendors/imgui/imgui.h"
#include "src/Vendors/imgui/ImGuizmo.h"
#include "src/Scene/Entities/ImGuiHelper.h"

#include "src/Rendering/Textures/Texture.h"
#include "src/Rendering/Textures/MaterialManager.h"

#include "src/Core/Physics/PhysicsManager.h"

#include "src/Vendors/glm/gtc/type_ptr.hpp"
#include "src/Vendors/glm/gtx/matrix_decompose.hpp"
#include "src/Resource/FontAwesome5.h"

#include "Dependencies/GLEW/include/GL/glew.h"

#include "src/Scene/Scene.h"
#include "src/Scene/Components/Components.h"
#include "src/Scene/Components/BoxCollider.h"
#include "src/Scene/Components/LuaScriptComponent.h"
#include "src/Scene/Components/WrenScriptComponent.h"
#include "src/Scene/Systems/QuakeMapBuilder.h"
#include "src/Scene/Components/LightComponent.h"
#include "UIComponents/Viewport.h"
#include <src/Resource/Prefab.h>
#include <src/Scene/Components/PrefabComponent.h>
#include <src/Rendering/Shaders/ShaderManager.h>
#include "src/Rendering/Renderer.h"
#include <src/Scene/Components/InterfaceComponent.h>
#include "src/Core/Input.h"

namespace Nuake {
    Ref<UI::UserInterface> userInterface;
    ImFont* normalFont;
    ImFont* boldFont;
    ImFont* EditorInterface::bigIconFont;
    void EditorInterface::Init()
    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_AutoHideTabBar;
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetWorkPos());
        ImGui::SetNextWindowSize(viewport->GetWorkSize());
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpaceOverViewport(viewport, dockspace_flags);
        //this->filesystem = FileSystemUI();
    }


    ImVec2 LastSize = ImVec2();
    void EditorInterface::DrawViewport()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        std::string name = ICON_FA_GAMEPAD + std::string(" Scene");
        if (ImGui::Begin(name.c_str()))
        {


            ImGui::PopStyleVar();
            Overlay();
            ImGuizmo::BeginFrame();

            ImGuizmo::SetOrthographic(false);
            ImVec2 regionAvail = ImGui::GetContentRegionAvail();
            Vector2 viewportPanelSize = glm::vec2(regionAvail.x, regionAvail.y);

            Ref<FrameBuffer> framebuffer = Engine::GetCurrentWindow()->GetFrameBuffer();
            if (framebuffer->GetSize() != viewportPanelSize)
                framebuffer->QueueResize(viewportPanelSize);

            Ref<Texture> texture = framebuffer->GetTexture();
            ImGui::Image((void*)texture->GetID(), regionAvail, ImVec2(0, 1), ImVec2(1, 0));

            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

            if (m_DrawGrid && !Engine::IsPlayMode)
            {
                ImGuizmo::DrawGrid(glm::value_ptr(Engine::GetCurrentScene()->GetCurrentCamera()->GetTransform()),
                    glm::value_ptr(Engine::GetCurrentScene()->GetCurrentCamera()->GetPerspective()),
                    glm::value_ptr(glm::identity<glm::mat4>()), 100.f);
            }

            if (m_IsEntitySelected && !Engine::IsPlayMode)
            {
                TransformComponent& tc = m_SelectedEntity.GetComponent<TransformComponent>();
                ParentComponent& parent = m_SelectedEntity.GetComponent<ParentComponent>();
                Matrix4 oldTransform = tc.GetGlobalTransform();
                Vector3 oldRotation = tc.Rotation;
                ImGuizmo::Manipulate(
                    glm::value_ptr(Engine::GetCurrentScene()->GetCurrentCamera()->GetTransform()),
                    glm::value_ptr(Engine::GetCurrentScene()->GetCurrentCamera()->GetPerspective()),
                    CurrentOperation, CurrentMode, glm::value_ptr(oldTransform), 0, 0
                );

                if (ImGuizmo::IsUsing())
                {
                    Vector3 globalPos = Vector3();
                    Entity currentParent = m_SelectedEntity;
                    if (parent.HasParent)
                    {
                        Matrix4 inverseParent = glm::inverse(parent.Parent.GetComponent<TransformComponent>().GlobalTransform);
                        oldTransform *= inverseParent;
                    }

                    Vector3 scale;
                    glm::quat rotation;
                    Vector3 translation;
                    Vector3 skew;
                    Vector4 perspective;
                    glm::decompose(oldTransform, scale, rotation, translation, skew, perspective);

                    rotation = glm::conjugate(rotation);
                    Vector3 euler = glm::eulerAngles(rotation);

                    if(CurrentOperation == ImGuizmo::TRANSLATE)
                        tc.Translation = translation;
                    if (CurrentOperation == ImGuizmo::ROTATE)
                    {
                        float signX = 1.0f;
                        if (oldRotation.x < 0.0f) signX = -1.0f;
                        float signY = 1.0f;
                        if (oldRotation.y < 0.0f) signY = -1.0f;
                        float signZ = 1.0f;
                        if (oldRotation.z < 0.0f) signZ = -1.0f;

                        tc.Rotation = glm::vec3(glm::degrees(euler.x),
                           glm::degrees(euler.y),
                           glm::degrees(euler.z));
                    }
                        
                    if (CurrentOperation == ImGuizmo::SCALE)
                        tc.Scale = scale;
                }
            }
        }
        else
        {
            ImGui::PopStyleVar();
        }
        ImGui::End();
    }

    static int selected = 0;
    Entity QueueDeletion;
    void EditorInterface::DrawEntityTree(Entity e)
    {
        ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

		NameComponent& nameComponent = e.GetComponent<NameComponent>();
		std::string name = nameComponent.Name;
        ParentComponent& parent = e.GetComponent<ParentComponent>();

        if (m_SelectedEntity == e)
            base_flags |= ImGuiTreeNodeFlags_Selected;

        ImGui::TableNextColumn();
        

        ImGui::TableNextColumn();

        // Write in normal font.
        ImGui::PushFont(normalFont);
        // If has no childrens draw tree node leaf
        if (parent.Children.size() <= 0)
            base_flags |= ImGuiTreeNodeFlags_Leaf;

		if(nameComponent.IsPrefab && e.HasComponent<PrefabComponent>())
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0,255,0,255));

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 8));

        bool open = ImGui::TreeNodeEx(name.c_str(), base_flags);
        ImGui::PopStyleVar();
		if(nameComponent.IsPrefab && e.HasComponent<PrefabComponent>())
			ImGui::PopStyleColor();
		else
			if (ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload("ENTITY", (void*)&e, sizeof(Entity));
				ImGui::Text(name.c_str());
				ImGui::EndDragDropSource();
			}

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
            {
                Entity payload_entity = *(const Entity*)payload->Data;

                // Check if entity is already parent.
                ParentComponent& parentPayload = payload_entity.GetComponent<ParentComponent>();
                if (!EntityContainsItself(payload_entity, e) && parentPayload.Parent != e && std::count(parent.Children.begin(), parent.Children.end(), payload_entity) == 0)
                {
                    if (parentPayload.HasParent)
                    {
                        // Erase remove idiom.
                        ParentComponent& childOfParent = parentPayload.Parent.GetComponent<ParentComponent>();
                        childOfParent.Children.erase(std::remove(childOfParent.Children.begin(), childOfParent.Children.end(), payload_entity), childOfParent.Children.end());
                    }

                    parentPayload.Parent = e;
                    parentPayload.HasParent = true;
                    parent.Children.push_back(payload_entity);
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::IsItemClicked())
        {
            m_SelectedEntity = e;
            m_IsEntitySelected = true;
        }

        if (m_IsEntitySelected)
        {
            if (ImGui::BeginPopupContextItem())
            {
                if (m_SelectedEntity.HasComponent<CameraComponent>())
                {
                    if (ImGui::Selectable("Focus camera"))
                    {
                        Engine::GetCurrentScene()->m_EditorCamera->Translation = m_SelectedEntity.GetComponent<TransformComponent>().Translation;
                        Engine::GetCurrentScene()->m_EditorCamera->SetDirection(m_SelectedEntity.GetComponent<CameraComponent>().CameraInstance->GetDirection());
                    }
                    ImGui::Separator();
                }

                if (ImGui::Selectable("Remove"))
                {
                    QueueDeletion = e;
                }


                if (ImGui::Selectable("Move to root"))
                {
                    auto& p = m_SelectedEntity.GetComponent<ParentComponent>();
                    if (p.HasParent)
                    {
                        auto& pp = p.Parent.GetComponent<ParentComponent>();
                        pp.RemoveChildren(m_SelectedEntity);
                        p.HasParent = false;
                    }
                }
                if (ImGui::Selectable("Save as new prefab"))
                {
                    Ref<Prefab> newPrefab = Prefab::CreatePrefabFromEntity(m_SelectedEntity);
                    std::string savePath = FileDialog::SaveFile("*.prefab");
                    newPrefab->SaveAs(savePath);
                    m_SelectedEntity.AddComponent<PrefabComponent>().PrefabInstance = newPrefab;
                }
                ImGui::EndPopup();
            }
        }
        if (open)
        {
            
            // Caching list to prevent deletion while iterating.
            std::vector<Entity> childrens = parent.Children;
            for (auto c : childrens)
                DrawEntityTree(c);

            ImGui::TreePop();
        }
         
        ImGui::PopFont();
    }

    void EditorInterface::DrawSceneTree()
    {
        Ref<Scene> scene = Engine::GetCurrentScene();
          
        if (!scene)
            return;

        if (ImGui::Begin("Environnement"))
        {
            const Ref<Environment> env = Engine::GetCurrentScene()->GetEnvironment();
            if (ImGui::CollapsingHeader("Procedural Sky", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::DragFloat("Sun Intensity", &env->ProceduralSkybox->SunIntensity, 0.1f, 0.0f);

                Vector3 currentDirection = env->ProceduralSkybox->SunDirection;
                ImGuiHelper::DrawVec3("Sun Direction", &currentDirection);
                env->ProceduralSkybox->SunDirection = glm::normalize(currentDirection);

                Vector3 mieScattering = env->ProceduralSkybox->MieScattering * 10000.0f;
                ImGuiHelper::DrawVec3("Mie Scattering", &mieScattering);
                env->ProceduralSkybox->MieScattering = mieScattering / 10000.0f;
            }
            if (ImGui::CollapsingHeader("HDR", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::DragFloat("Exposure", &env->Exposure, .01f, 0.0f, 10.0f);
                ImGui::DragFloat("Gamma", &env->Gamma, 0.1f, 0.0f, 10.0f);
            }
            if (ImGui::CollapsingHeader("Bloom", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Checkbox("Enabled", &env->BloomEnabled);
                float threshold = env->mBloom->GetThreshold();
                ImGui::DragFloat("Threshold", &threshold, 0.01f, 0.0f, 500.0f);
                env->mBloom->SetThreshold(threshold);

                int iteration = env->mBloom->GetIteration();
                ImGui::DragInt("Iteration", &iteration, 1.0f, 0, 8);
                env->mBloom->SetIteration(iteration);
            }
            if (ImGui::CollapsingHeader("Fog", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Checkbox("Enabled", &env->VolumetricEnabled);
                ImGui::DragFloat("Volumetric Scattering", &env->VolumetricFog, .01f, 0.0f, 1.0f);
                ImGui::DragFloat("Volumetric Step Count", &env->VolumetricStepCount, 1.f, 0.0f);
            }
            if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::DragFloat("Exposure", &Engine::GetCurrentScene()->m_EditorCamera->Exposure, .01f, 0.0f, 5.0f);
            }
        }
        ImGui::End();

        std::string title = ICON_FA_TREE + std::string(" Hierarchy");
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        if (ImGui::Begin(title.c_str()))
        {
            // Buttons to add and remove entity.
            if(ImGui::BeginChild("Buttons", ImVec2(300, 30), false))
            {
                // Add entity.
                if (ImGui::Button(ICON_FA_PLUS))
                    Engine::GetCurrentScene()->CreateEntity("Entity");

               //// Remove Entity
               //if (ImGui::Button("Remove"))
               //{
               //    scene->DestroyEntity(m_SelectedEntity);
               //
               //    // Unselect delted entity.
               //    m_SelectedEntity = scene->GetAllEntities().at(0);
               //}
            }
            ImGui::EndChild();

            ImGui::Separator();
            // Draw a tree of entities.
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(26.f / 255.0f, 26.f / 255.0f, 26.f / 255.0f, 1));
            if (ImGui::BeginChild("Scene tree", ImGui::GetContentRegionAvail(), false))
            {
                if (ImGui::BeginTable("entity_table", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoBordersInBody))
                {
                    std::string icon = ICON_FA_EYE;
                    ImGui::TableSetupColumn(("    " + icon).c_str(), ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_IndentDisable | ImGuiTableColumnFlags_WidthFixed, 16);
                    ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_IndentEnable);
                    ImGui::TableHeadersRow();
                    ImGui::TableNextRow();

                    std::vector<Entity> entities = scene->GetAllEntities();
                    for (Entity e : entities)
                    {
                        ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
                        std::string name = e.GetComponent<NameComponent>().Name;
                        // If selected add selected flag.
                        if (m_SelectedEntity == e)
                            base_flags |= ImGuiTreeNodeFlags_Selected;

                        // Write in normal font.
                        ImGui::PushFont(normalFont);

                        // Small icons + name.
                        std::string label = ICON_FA_CIRCLE + std::string(" ") + name;

                        // Draw all entity without parents.
                        if (!e.GetComponent<ParentComponent>().HasParent)
                        {
                            // Recursively draw childrens.
                            DrawEntityTree(e);
                        }

                        // Pop font.
                        ImGui::PopFont();

                        // Right click menu
                        //if (ImGui::BeginPopupContextItem())
                        //    ImGui::EndPopup();
                    }
                }

                ImGui::EndTable();
            }
			ImGui::EndChild();
            ImGui::PopStyleColor();
            
            if (ImGui::BeginDragDropTarget()) // Drag n drop new prefab file into scene tree
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Prefab"))
                {
                    char* file = (char*)payload->Data;
                    std::string fullPath = std::string(file, 256);
                    std::string relPath = FileSystem::AbsoluteToRelative(fullPath);

                    Entity newEntity = Engine::GetCurrentScene()->CreateEntity("New prefab Entity");
                    PrefabComponent& prefabComponent = newEntity.AddComponent<PrefabComponent>();
                    prefabComponent.PrefabInstance = Prefab::New(relPath);
                }
                ImGui::EndDragDropTarget();
            }

            // Deleted entity queue
            if (QueueDeletion.GetHandle() != -1)
            {
                Engine::GetCurrentScene()->DestroyEntity(QueueDeletion);

                if (m_SelectedEntity == QueueDeletion)
                    m_IsEntitySelected = false;

                QueueDeletion = Entity{ (entt::entity)-1, scene.get() };
            }
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void EditorInterface::DrawEntityPropreties()
    {
        if (ImGui::Begin("Propreties"))
        {
            if (!m_IsEntitySelected)
            {
                std::string text = "No entity selected";
                auto windowWidth = ImGui::GetWindowSize().x;
                auto windowHeight = ImGui::GetWindowSize().y;

                auto textWidth = ImGui::CalcTextSize(text.c_str()).x;
                auto textHeight = ImGui::CalcTextSize(text.c_str()).y;
                ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
                ImGui::SetCursorPosY((windowHeight - textHeight) * 0.5f);

                ImGui::PushFont(boldFont);
                ImGui::Text(text.c_str());
                ImGui::PopFont();
                ImGui::End();
                return;
            }

            if (m_SelectedEntity.HasComponent<NameComponent>()) {
                auto& name = m_SelectedEntity.GetComponent<NameComponent>().Name;

                char buffer[256];
                memset(buffer, 0, sizeof(buffer));
                std::strncpy(buffer, name.c_str(), sizeof(buffer));
                ImGui::Text("Name: ");
                ImGui::SameLine();
                if (ImGui::InputText("##Name", buffer, sizeof(buffer)))
                {
                    name = std::string(buffer);
                }
                ImGui::SameLine();
                if (ImGui::Button("Add component"))
                {
                    ImGui::OpenPopup("add_component_popup");

                }
                if (ImGui::BeginPopup("add_component_popup"))
                {
                    if (ImGui::MenuItem("Wren Script") && !m_SelectedEntity.HasComponent<WrenScriptComponent>())
                        m_SelectedEntity.AddComponent<WrenScriptComponent>();
                    ImGui::Separator();
                    if (ImGui::MenuItem("Camera Component") && !m_SelectedEntity.HasComponent<CameraComponent>())
                        m_SelectedEntity.AddComponent<CameraComponent>();
                    ImGui::Separator();
                    if (ImGui::MenuItem("Light Component") && !m_SelectedEntity.HasComponent<LightComponent>())
                        m_SelectedEntity.AddComponent<LightComponent>();
                    ImGui::Separator();
                    if (ImGui::MenuItem("Mesh Component") && !m_SelectedEntity.HasComponent<MeshComponent>())
                        m_SelectedEntity.AddComponent<MeshComponent>();
                    if (ImGui::MenuItem("Quake map Component") && !m_SelectedEntity.HasComponent<QuakeMapComponent>())
                        m_SelectedEntity.AddComponent<QuakeMapComponent>();
                    ImGui::Separator();
                    if (ImGui::MenuItem("Character controller") && !m_SelectedEntity.HasComponent<CharacterControllerComponent>())
                        m_SelectedEntity.AddComponent<CharacterControllerComponent>();
                    if (ImGui::MenuItem("Rigidbody Component") && !m_SelectedEntity.HasComponent<RigidBodyComponent>())
                    {
                        m_SelectedEntity.AddComponent<RigidBodyComponent>();
                        auto transformComponent = m_SelectedEntity.GetComponent<TransformComponent>();
                    }
                    if (ImGui::MenuItem("Box collider Component") && !m_SelectedEntity.HasComponent<BoxColliderComponent>())
                    {
                        m_SelectedEntity.AddComponent<BoxColliderComponent>();
                    }
                    if (ImGui::MenuItem("Sphere collider Component") && !m_SelectedEntity.HasComponent<SphereColliderComponent>())
                    {
                        m_SelectedEntity.AddComponent<SphereColliderComponent>();
                    }
                    if (ImGui::MenuItem("Mesh collider Component") && !m_SelectedEntity.HasComponent<MeshColliderComponent>())
                    {
                        m_SelectedEntity.AddComponent<MeshColliderComponent>();
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Interface Component") && !m_SelectedEntity.HasComponent<InterfaceComponent>())
                    {
                        m_SelectedEntity.AddComponent< InterfaceComponent>();
                    }

                    // your popup code
                    ImGui::EndPopup();
                }
                ImGui::Separator();
            }

            
            std::string iconTransform = ICON_FA_MAP_MARKER;

            ImGui::PushFont(boldFont);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 8));
            bool headerOpened = ImGui::CollapsingHeader(" TRANSFORM", ImGuiTreeNodeFlags_DefaultOpen);
            if (headerOpened)
            {
                ImGui::PopFont();
                ImGui::PopStyleVar();
                if (ImGui::BeginTable("TransformTable", 3, ImGuiTableFlags_BordersInner))
                {
                    TransformComponent& component = m_SelectedEntity.GetComponent<TransformComponent>();

                    ImGui::TableSetupColumn("name", 0, 0.3);
                    ImGui::TableSetupColumn("set", 0, 0.6);
                    ImGui::TableSetupColumn("reset", 0, 0.1);

                    ImGui::TableNextColumn();
                    ImGui::Text("Translation");
                    ImGui::TableNextColumn();
                    ImGuiHelper::DrawVec3("Translation", &component.Translation);
                    ImGui::TableNextColumn();

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetTranslation = ICON_FA_UNDO + std::string("##ResetTranslation");
                    if (ImGui::Button(resetTranslation.c_str())) component.Translation = Vector3(0, 0, 0);
                    ImGui::PopStyleColor();

                    ImGui::TableNextColumn();
                    ImGui::Text("Rotation");

                    ImGui::TableNextColumn();
                    ImGuiHelper::DrawVec3("Rotation", &component.Rotation);

                    ImGui::TableNextColumn();
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    std::string resetRotation = ICON_FA_UNDO + std::string("##ResetRotation");
                    if (ImGui::Button(resetRotation.c_str())) component.Rotation = Vector3(0, 0, 0);
                    ImGui::PopStyleColor();

                    ImGui::TableNextColumn();
                    ImGui::Text("Scale");

                    ImGui::TableNextColumn();
                    ImGuiHelper::DrawVec3("Scale", &component.Scale);

                    ImGui::TableNextColumn();
                    std::string resetScale = ICON_FA_UNDO + std::string("##ResetScale");
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                    if (ImGui::Button(resetScale.c_str())) component.Scale = Vector3(1, 1, 1);
                    ImGui::PopStyleColor();

                    ImGui::EndTable();
                }
                //ImGui::InputText("Name:", selectedEntity->m_Name.data(), 12);

            }
            else
            {
                ImGui::PopStyleVar();
                ImGui::PopFont();
            }

            ImGui::PopStyleVar();

            if (m_SelectedEntity.HasComponent<InterfaceComponent>())
            {
                ImGui::PushFont(boldFont);
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 8));

                headerOpened = ImGui::CollapsingHeader("Interface", ImGuiTreeNodeFlags_DefaultOpen);
                if (headerOpened)
                {
                    ImGui::PopFont();
                    ImGui::PopStyleVar();
                    InterfaceComponent& component = m_SelectedEntity.GetComponent<InterfaceComponent>();
                    std::string& path = component.Path;
                    char pathBuffer[256];
                    memset(pathBuffer, 0, sizeof(pathBuffer));
                    std::strncpy(pathBuffer, path.c_str(), sizeof(pathBuffer));

                    ImGui::Text("Interface: ");
                    ImGui::SameLine();

                    if (ImGui::InputText("##InterfacePath", pathBuffer, sizeof(pathBuffer)))
                        path = FileSystem::AbsoluteToRelative(std::string(pathBuffer));

                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Interface"))
                        {
                            char* file = (char*)payload->Data;
                            std::string fullPath = std::string(file, 256);
                            path = FileSystem::AbsoluteToRelative(fullPath);
                            component.SetInterface(path);
                        }
                        ImGui::EndDragDropTarget();
                    }
                }
                else
                {
                    ImGui::PopStyleVar();
                    ImGui::PopFont();
                }

                ImGui::PopStyleVar();
            }

            if (m_SelectedEntity.HasComponent<MeshComponent>()) 
            {
                std::string icon = ICON_FA_MALE;
                if (ImGui::CollapsingHeader((icon + " " + "Mesh").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::TextColored(ImGui::GetStyleColorVec4(1), "Mesh properties");
                    auto& component = m_SelectedEntity.GetComponent<MeshComponent>();
                    // Path
                    std::string path = component.ModelPath;
                    char pathBuffer[256];

                    memset(pathBuffer, 0, sizeof(pathBuffer));
                    std::strncpy(pathBuffer, path.c_str(), sizeof(pathBuffer));

                    std::string oldPath = component.ModelPath;
                    ImGui::Text("Model: ");
                    ImGui::SameLine();
                    if (ImGui::InputText("##ModelPath", pathBuffer, sizeof(pathBuffer)))
                        path = FileSystem::AbsoluteToRelative(std::string(pathBuffer));
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Model"))
                        {
                            char* file = (char*)payload->Data;
                            std::string fullPath = std::string(file, 256);
                            path = FileSystem::AbsoluteToRelative(fullPath);
                        }
                        ImGui::EndDragDropTarget();
                    }

                    if (component.ModelPath != path)
                    {
                        component.ModelPath = path;
                        component.LoadModel();
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("Reimport"))
                    {
                        component.LoadModel();
                    }

                    ImGui::Indent(16.0f);
                    if (ImGui::CollapsingHeader("Meshes"))
                    {
                        ImGui::Indent(16.0f);
                        uint16_t index = 0;
                        for (auto& m : component.meshes)
                        {
                            if (ImGui::CollapsingHeader(std::to_string(index).c_str()))
                            {
                                std::string materialName = "No material";
                                if (m->m_Material)
                                    materialName = m->m_Material->GetName();

                                ImGui::Indent(16.0f);
                                if (ImGui::CollapsingHeader(materialName.c_str()))
                                {
                                    //if (ImGui::BeginChild("Material child", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysAutoResize))
                                    //{
                                    ImGui::Indent(16.0f);
                                    DrawMaterialEditor(m->m_Material);
                                    //}
                                    //ImGui::EndChild();
                                }

                                if (ImGui::BeginDragDropTarget())
                                {
                                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Material"))
                                    {
                                        char* file = (char*)payload->Data;
                                        std::string fullPath = std::string(file, 256);
                                        path = FileSystem::AbsoluteToRelative(fullPath);
                                    }
                                    ImGui::EndDragDropTarget();
                                }
                            }

                            index++;
                        }
                        
                    }

                    ImGui::Separator();
                }

            }

            if (m_SelectedEntity.HasComponent<LightComponent>()) 
            {
                ImGui::PushFont(boldFont);
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 8));

                bool headerOpened = ImGui::CollapsingHeader(" LIGHT", ImGuiTreeNodeFlags_DefaultOpen);
                if (headerOpened)
                {
                    ImGui::PopFont();
                    ImGui::PopStyleVar();
                    if (ImGui::BeginTable("TransformTable", 3, ImGuiTableFlags_BordersInner))
                    {
                        LightComponent& component = m_SelectedEntity.GetComponent<LightComponent>();

                        ImGui::TableSetupColumn("name", 0, 0.3);
                        ImGui::TableSetupColumn("set", 0, 0.6);
                        ImGui::TableSetupColumn("reset", 0, 0.1);

                        ImGui::TableNextColumn();
                        ImGui::Text("Color");
                        ImGui::TableNextColumn();
                        ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
                        ImGui::ColorEdit3("##lightcolor", &component.Color.r, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                        ImGui::PopItemWidth();
                        ImGui::TableNextColumn();

                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                        std::string resetColor = ICON_FA_UNDO + std::string("##ResetTranslation");
                        if (ImGui::Button(resetColor.c_str())) component.Color = Vector3(1, 1, 1);
                        ImGui::PopStyleColor();

                        ImGui::TableNextColumn();
                        ImGui::Text("Strength");

                        ImGui::TableNextColumn();
                        ImGui::SliderFloat("Strength", &component.Strength, 0.0f, 100.0f);

                        ImGui::TableNextColumn();
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                        std::string resetStrength = ICON_FA_UNDO + std::string("##resetStrength");
                        if (ImGui::Button(resetStrength.c_str())) component.Strength = 10.0f;
                        ImGui::PopStyleColor();

                        ImGui::TableNextColumn();
                        ImGui::Text("Cast Shadows");

                        ImGui::TableNextColumn();
                        
                        // This will create framebuffers and textures.
                        bool castShadows = component.CastShadows;
                        ImGui::Checkbox("##CastShadows", &component.CastShadows);
                        if (castShadows != component.CastShadows)
                            component.SetCastShadows(castShadows);


                        ImGui::TableNextColumn();
                        std::string resetShadow = ICON_FA_UNDO + std::string("##resetShadow");
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                        if (ImGui::Button(resetShadow.c_str())) component.CastShadows = false;
                        ImGui::PopStyleColor();

                        ImGui::TableNextColumn();
                        ImGui::Text("Type");
                        ImGui::TableNextColumn();
                        const char* types[] = { "Directional", "Point", "Spot" };
                        static const char* current_item = types[component.Type];

                        if (ImGui::BeginCombo("Type", current_item)) // The second parameter is the label previewed before opening the combo.
                        {
                            for (int n = 0; n < IM_ARRAYSIZE(types); n++)
                            {
                                bool is_selected = (current_item == types[n]); // You can store your selection however you want, outside or inside your objects
                                if (ImGui::Selectable(types[n], is_selected)) 
                                {
                                    current_item = types[n];
                                    component.Type = (LightType)n;
                                }
                                if (is_selected)
                                    ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                            }
                            ImGui::EndCombo();
                        }

                        ImGui::TableNextColumn();
                        std::string resetType = ICON_FA_UNDO + std::string("##resetType");
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                        if (ImGui::Button(resetShadow.c_str())) component.Type = Point;
                        ImGui::PopStyleColor();


                        if (component.Type == Directional)
                        {
                            ImGui::TableNextColumn();
                            ImGui::Text("Is Volumetric");

                            ImGui::TableNextColumn();
                            ImGui::Checkbox("##Volumetric", &component.IsVolumetric);

                            ImGui::TableNextColumn();
                            std::string resetIsVolumetric = ICON_FA_UNDO + std::string("##IsVolumetric");
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                            if (ImGui::Button(resetIsVolumetric.c_str())) component.IsVolumetric = false;
                            ImGui::PopStyleColor();

                            ImGui::TableNextColumn();
                            ImGui::Text("Sync Sky Direction");

                            ImGui::TableNextColumn();
                            ImGui::Checkbox("##SyncSky", &component.SyncDirectionWithSky);

                            ImGui::TableNextColumn();
                            std::string resetSyncDirectionWithSky = ICON_FA_UNDO + std::string("##resetSyncDirectionWithSky");
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                            if (ImGui::Button(resetSyncDirectionWithSky.c_str())) component.SyncDirectionWithSky = false;
                            ImGui::PopStyleColor();

                            ImGui::TableNextColumn();
                            ImGui::Text("Direction");
                            ImGui::TableNextColumn();
                            ImGuiHelper::DrawVec3("Direction", &component.Direction);
                            ImGui::TableNextColumn();

                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0));
                            std::string resetDirection = ICON_FA_UNDO + std::string("##resetDirection");
                            if (ImGui::Button(resetDirection.c_str())) component.Direction = Vector3(0, -1, 0);
                            ImGui::PopStyleColor();
                        }
                        ImGui::EndTable();
                    }
                }
                else
                {
                    ImGui::PopStyleVar();
                    ImGui::PopFont();
                }
                ImGui::PopStyleVar();
            }

            if (m_SelectedEntity.HasComponent<WrenScriptComponent>()) {
                std::string icon = ICON_FA_FILE;
                if (ImGui::CollapsingHeader((icon + " " + "Wren Script").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::TextColored(ImGui::GetStyleColorVec4(1), "Script properties");
                    auto& component = m_SelectedEntity.GetComponent<WrenScriptComponent>();

                    // Path
                    std::string path = component.Script;
                    char pathBuffer[256];

                    memset(pathBuffer, 0, sizeof(pathBuffer));
                    std::strncpy(pathBuffer, path.c_str(), sizeof(pathBuffer));

                    ImGui::Text("Script: ");
                    ImGui::SameLine();
                    if (ImGui::InputText("##ScriptPath", pathBuffer, sizeof(pathBuffer)))
                        path = FileSystem::AbsoluteToRelative(std::string(pathBuffer));
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Script"))
                        {
                            char* file = (char*)payload->Data;
                            std::string fullPath = std::string(file, 256);
                            path = FileSystem::AbsoluteToRelative(fullPath);
                        }
                        ImGui::EndDragDropTarget();
                    }


                    component.Script = path;

                    // Class
                    std::string module = component.Class;

                    char classBuffer[256];

                    memset(classBuffer, 0, sizeof(classBuffer));
                    std::strncpy(classBuffer, module.c_str(), sizeof(classBuffer));

                    ImGui::Text("Class: ");
                    ImGui::SameLine();
                    if (ImGui::InputText("##ScriptModule", classBuffer, sizeof(classBuffer)))
                        module = std::string(classBuffer);

                    component.Class = module;
                    ImGui::Separator();
                }
            }

            if (m_SelectedEntity.HasComponent<CameraComponent>()) {

                std::string icon = ICON_FA_LIGHTBULB;
                if (ImGui::CollapsingHeader((icon + " " + "Camera").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::TextColored(ImGui::GetStyleColorVec4(1), "Camera properties");
                    m_SelectedEntity.GetComponent<CameraComponent>().DrawEditor();
                    ImGui::Separator();
                }

            }

            if (m_SelectedEntity.HasComponent<CharacterControllerComponent>())
            {
                if (ImGui::CollapsingHeader("Character controller", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::TextColored(ImGui::GetStyleColorVec4(1), "Character controller properties");
                    auto& c = m_SelectedEntity.GetComponent<CharacterControllerComponent>();
                    ImGui::InputFloat("Height", &c.Height);
                    ImGui::InputFloat("Radius", &c.Radius);
                    ImGui::InputFloat("Mass", &c.Mass);
                    ImGui::Separator();
                }
            }
            if (m_SelectedEntity.HasComponent<RigidBodyComponent>())
            {
                std::string icon = ICON_FA_BOWLING_BALL;
                if (ImGui::CollapsingHeader((icon + " Rigidbody").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::TextColored(ImGui::GetStyleColorVec4(1), "Rigidbody properties");
                    RigidBodyComponent& rbComponent = m_SelectedEntity.GetComponent<RigidBodyComponent>();
                    ImGui::DragFloat("Mass", &rbComponent.mass, 0.1, 0.0);
                    ImGui::Separator();
                }
            }
            if (m_SelectedEntity.HasComponent<BoxColliderComponent>())
            {
                std::string icon = ICON_FA_BOX;
                if (ImGui::CollapsingHeader((icon + " Box collider").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::TextColored(ImGui::GetStyleColorVec4(1), "Box collider properties");
                    BoxColliderComponent& component = m_SelectedEntity.GetComponent<BoxColliderComponent>();
                    ImGuiHelper::DrawVec3("Size", &component.Size);
                    ImGui::Checkbox("Is trigger", &component.IsTrigger);

                    ImGui::Separator();
                }
            }
            if (m_SelectedEntity.HasComponent<SphereColliderComponent>())
            {
                std::string icon = ICON_FA_CIRCLE;
                if (ImGui::CollapsingHeader((icon + " Sphere collider").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::TextColored(ImGui::GetStyleColorVec4(1), "Sphere properties");
                    SphereColliderComponent& component = m_SelectedEntity.GetComponent<SphereColliderComponent>();
                    ImGui::DragFloat("Radius", &component.Radius, 0.1f, 0.0f, 100.0f);
                    ImGui::Checkbox("Is trigger", &component.IsTrigger);

                    ImGui::Separator();
                }
            }
            if (m_SelectedEntity.HasComponent<QuakeMapComponent>())
            {
                
                std::string icon = ICON_FA_BROOM;
                if (ImGui::CollapsingHeader((icon + " " + "Quake map").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::TextColored(ImGui::GetStyleColorVec4(1), "Quake map properties");
                    auto& component = m_SelectedEntity.GetComponent<QuakeMapComponent>();
                    std::string path = component.Path;


                    char pathBuffer[256];
                    memset(pathBuffer, 0, sizeof(pathBuffer));
                    std::strncpy(pathBuffer, path.c_str(), sizeof(pathBuffer));
                    ImGui::Text("Map file: ");
                    ImGui::SameLine();
                    if (ImGui::InputText("##MapPath", pathBuffer, sizeof(pathBuffer)))
                    {
                        path = std::string(pathBuffer);
                    }

                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Map"))
                        {
                            char* file = (char*)payload->Data;
                            std::string fullPath = std::string(file, 256);
                            path = FileSystem::AbsoluteToRelative(fullPath);
                        }
                        ImGui::EndDragDropTarget();
                    }

                    component.Path = path;

                    ImGui::InputFloat("Scale factor", &component.ScaleFactor, 0.01f, 0.1f);

                    ImGui::Checkbox("Build collisions", &component.HasCollisions);
                    if (ImGui::Button("Build Geometry"))
                    {
                        QuakeMapBuilder mapBuilder;
                        mapBuilder.BuildQuakeMap(m_SelectedEntity);
                    }
                    ImGui::Separator();
                }
            }

        }
        ImGui::End();
    }

    void EditorInterface::DrawGizmos()
    {
        Ref<Scene> scene = Engine::GetCurrentScene();

        if (!m_IsEntitySelected)
            return;
    }

    void EditorInterface::EditorInterfaceDrawFiletree(Ref<Directory> dir)
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

    void EditorInterface::DrawFileSystem()
    {
        Ref<Directory> rootDirectory = FileSystem::GetFileTree();
        if (!rootDirectory)
            return;
    }

    void EditorInterface::DrawDirectory(Ref<Directory> directory)
    {
        ImGui::PushFont(bigIconFont);
        std::string id = ICON_FA_FOLDER + std::string("##") + directory->name;
        if (ImGui::Button(id.c_str(), ImVec2(100, 100)))
            m_CurrentDirectory = directory;
        ImGui::Text(directory->name.c_str());
        ImGui::PopFont();
    }

    bool EditorInterface::EntityContainsItself(Entity source, Entity target)
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

    void EditorInterface::DrawFile(Ref<File> file)
    {
        ImGui::PushFont(bigIconFont);
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
            else if (file->Type == ".map")
                icon = ICON_FA_BROOM;
            else if (file->Type == ".ogg" || file->Type == ".mp3" || file->Type == ".wav" || file->Type == ".flac")
                icon = ICON_FA_FILE_AUDIO;
            else if (file->Type == ".cpp" || file->Type == ".h" || file->Type == ".cs" || file->Type == ".py" || file->Type == ".lua")
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

    void EditorInterface::DrawDirectoryExplorer()
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
                if (!m_CurrentDirectory) 
                {
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
            }

            ImGui::EndTable();
        }
        ImGui::End();
    }

    bool LogErrors = true;
    bool LogWarnings = true;
    bool LogDebug = true;
    void EditorInterface::DrawLogger()
    {
        if (ImGui::Begin("Logger"))
        {
            ImGui::Checkbox("Errors", &LogErrors);
            ImGui::SameLine();
            ImGui::Checkbox("Warning", &LogWarnings);
            ImGui::SameLine();
            ImGui::Checkbox("Debug", &LogDebug);

            //ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            //if (ImGui::BeginChild("Log window", ImGui::GetContentRegionAvail(), false))
            //{
                //ImGui::PopStyleVar();
            ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

            if (ImGui::BeginTable("LogTable", 3, flags))
            {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("Severity");
                ImGui::TableSetupColumn("Time");
                ImGui::TableSetupColumn("Message");
                ImGui::TableHeadersRow();
                ImGui::TableNextColumn();
                for (auto& l : Logger::GetLogs())
                {
                    if (l.type == LOG_TYPE::VERBOSE && !LogDebug)
                        continue;
                    if (l.type == LOG_TYPE::WARNING && !LogWarnings)
                        continue;
                    if (l.type == LOG_TYPE::CRITICAL && !LogErrors)
                        continue;
                    ImGui::Text("-");
                    ImGui::TableNextColumn();
                    ImGui::Text(l.time.c_str());
                    ImGui::TableNextColumn();
                    ImGui::TextWrapped(l.message.c_str());

                    ImGui::TableNextColumn();
                }

                ImGui::EndTable();
            }


            //ImGui::EndChild();
        //}


        }
        ImGui::End();
    }



    void EditorInterface::Overlay()
    {
        // FIXME-VIEWPORT: Select a default viewport
        const float DISTANCE = 10.0f;
        int corner = 0;
        ImGuiIO& io = ImGui::GetIO();
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        if (corner != -1)
        {
            window_flags |= ImGuiWindowFlags_NoMove;
            ImGuiViewport* viewport = ImGui::GetWindowViewport();
            ImVec2 work_area_pos = ImGui::GetCurrentWindow()->Pos;   // Instead of using viewport->Pos we use GetWorkPos() to avoid menu bars, if any!
            ImVec2 work_area_size = ImGui::GetCurrentWindow()->Size;
            ImVec2 window_pos = ImVec2((corner & 1) ? (work_area_pos.x + work_area_size.x - DISTANCE) : (work_area_pos.x + DISTANCE), (corner & 2) ? (work_area_pos.y + work_area_size.y - DISTANCE) : (work_area_pos.y + DISTANCE));
            ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
            ImGui::SetNextWindowViewport(viewport->ID);
        }
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 32.0f);
        if (ImGui::Begin("Example: Simple overlay", &m_ShowOverlay, window_flags))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100);
            if (ImGui::Button(ICON_FA_HAND_POINTER)) CurrentOperation = ImGuizmo::OPERATION::TRANSLATE;
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ARROWS_ALT)) CurrentOperation = ImGuizmo::OPERATION::TRANSLATE;
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_SYNC_ALT)) CurrentOperation = ImGuizmo::OPERATION::ROTATE;
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_EXPAND_ALT)) CurrentOperation = ImGuizmo::OPERATION::SCALE;
            ImGui::PopStyleVar();

        }
        ImGui::PopStyleVar();
        ImGui::End();
        int corner2 = 1;

        window_flags |= ImGuiWindowFlags_NoMove;
        ImGuiViewport* viewport = ImGui::GetWindowViewport();
        ImVec2 work_area_pos = ImGui::GetCurrentWindow()->Pos;   // Instead of using viewport->Pos we use GetWorkPos() to avoid menu bars, if any!
        ImVec2 work_area_size = ImGui::GetCurrentWindow()->Size;
        ImVec2 window_pos = ImVec2((corner2 & 1) ? (work_area_pos.x + work_area_size.x - DISTANCE) : (work_area_pos.x + DISTANCE), (corner2 & 2) ? (work_area_pos.y + work_area_size.y - DISTANCE) : (work_area_pos.y + DISTANCE));
        ImVec2 window_pos_pivot = ImVec2((corner2 & 1) ? 1.0f : 0.0f, (corner2 & 2) ? 1.0f : 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 32.0f);
        if (ImGui::Begin("Controls", &m_ShowOverlay, window_flags))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100);
            if (ImGui::Button("...")) {};
            ImGui::PopStyleVar();
        }

        ImGui::PopStyleVar();
        ImGui::End();
    }

    void EditorInterface::DrawMaterialEditor(Ref<Material> material)
    {

        ImGui::Text("Flags");
        bool unlit = material->data.u_Unlit == 1;
        ImGui::Checkbox("Unlit", &unlit);
        material->data.u_Unlit = (int)unlit;
        if (ImGui::CollapsingHeader("Albedo", ImGuiTreeNodeFlags_DefaultOpen))
        {
            unsigned int textureID = 0;
            textureID = material->m_Albedo->GetID();


            ImGui::ColorEdit3("Color", &material->data.m_AlbedoColor.r);
            if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image1"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec2(2, 2), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
            {
                std::string texture = FileDialog::OpenFile("*.png | *.jpg");
                if (texture != "" && texture != material->m_Albedo->GetPath())
                    material->m_Albedo = TextureManager::Get()->GetTexture(texture);
            }

            ImGui::SameLine();
            bool isAlbedo = material->data.u_HasAlbedo == 1;
            ImGui::Checkbox("Use##1", &isAlbedo);
            material->data.u_HasAlbedo = (int)isAlbedo;

        }
        if (ImGui::CollapsingHeader("AO", ImGuiTreeNodeFlags_DefaultOpen))
        {
            unsigned int textureID = 0;
            if (material->HasAO())
                textureID = material->m_AO->GetID();
            if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image2"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec2(2, 2), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1)))
            {
                std::string texture = FileDialog::OpenFile("Image files (*.png) | *.png | Image files (*.jpg) | *.jpg");
                if (texture != "")
                {
                    m_SelectedMaterial->SetAO(TextureManager::Get()->GetTexture(texture));
                }
            }
            ImGui::SameLine();
            bool hasAO = material->data.u_HasAO == 1;
            ImGui::Checkbox("Use##1", &hasAO);
            material->data.u_HasAO = (int)hasAO;

            ImGui::SameLine();
            ImGui::DragFloat("Value##2", &material->data.u_AOValue, 0.01f, 0.0f, 1.0f);
        }
        if (ImGui::CollapsingHeader("Normal", ImGuiTreeNodeFlags_DefaultOpen))
        {
            unsigned int textureID = 0;
            if (material->HasNormal())
                textureID = material->m_Normal->GetID();
            if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image3"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec2(2, 2), ImVec4(0, 0, 0, 1), ImVec4(1, 1, 1, 1)))
            {
                std::string texture = FileDialog::OpenFile("*.png");
                if (texture != "")
                {
                    material->SetNormal(TextureManager::Get()->GetTexture(texture));
                }
            }
            //ImGui::SameLine();
            //ImGui::Checkbox("Use##3", &m_SelectedMaterial->data.u_HasNormal);
        }
        if (ImGui::CollapsingHeader("Metalness", ImGuiTreeNodeFlags_DefaultOpen))
        {
            unsigned int textureID = 0;
            if (material->HasMetalness())
                textureID = material->m_Metalness->GetID();
            if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image4"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec2(2, 2), ImVec4(0, 0, 0, 1), ImVec4(1, 1, 1, 1)))
            {
                std::string texture = FileDialog::OpenFile("*.png | *.jpg");
            }
            ImGui::SameLine();
            //ImGui::Checkbox("Use##4", &m_SelectedMaterial->data.u_HasMetalness);
            ImGui::SameLine();
            ImGui::DragFloat("Value##4", &material->data.u_MetalnessValue, 0.01f, 0.0f, 1.0f);
        }
        if (ImGui::CollapsingHeader("Roughness", ImGuiTreeNodeFlags_DefaultOpen))
        {
            unsigned int textureID = 0;
            if (material->HasRoughness())
                textureID = material->m_Roughness->GetID();
            if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image5"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec2(2, 2), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
            {
                std::string texture = FileDialog::OpenFile("*.png | *.jpg");
            }
            ImGui::SameLine();
            //ImGui::Checkbox("Use##5", &m_SelectedMaterial->data.u_HasRoughness);
            ImGui::SameLine();
            ImGui::DragFloat("Value##5", &material->data.u_RoughnessValue, 0.01f, 0.0f, 1.0f);
        }
    }


    void EditorInterface::DrawRessourceWindow()
    {
        std::map<std::string, Ref<Material>> materials = MaterialManager::Get()->GetAllMaterials();

        if (ImGui::Begin("Materials"))
        {
            int width = ImGui::GetWindowWidth();
            ImVec2 buttonSize = ImVec2(100, 100);
            int amount = (width / 100) - 1;
            if (amount > 0)
            {
                int i = 1;
                for (auto m : materials)
                {
                    unsigned int textureID = 0;
                    if (m.second->HasAlbedo())
                        textureID = m.second->m_Albedo->GetID();
                    std::string id = "materialButton" + std::to_string(i);
                    if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID(id.c_str()), (void*)textureID, ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0), ImVec2(2, 2), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
                    {
                        m_SelectedMaterial = m.second;
                        m_IsMaterialSelected = true;
                    }
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                    {
                        // Set payload to carry the index of our item (could be anything)
                        ImGui::SetDragDropPayload("MaterialName", &m.first, sizeof(int));

                        // Display preview (could be anything, e.g. when dragging an image we could decide to display
                        // the filename and a small preview of the image, etc.)
                        ImGui::Text("hello");
                        ImGui::EndDragDropSource();
                    }
                    if (i % amount != 0)
                        ImGui::SameLine();
                    i++;
                }
            }


        }
        ImGui::End();
        if (ImGui::Begin("Material property"))
        {
            if (m_IsMaterialSelected)
            {
                ImGui::Text("Flags");
                bool unlit = m_SelectedMaterial->data.u_Unlit == 1;
                ImGui::Checkbox("Unlit", &unlit);
                m_SelectedMaterial->data.u_Unlit = (int)unlit;
                if (ImGui::CollapsingHeader("Albedo", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    unsigned int textureID = 0;
                    if (m_SelectedMaterial->HasAlbedo())
                        textureID = m_SelectedMaterial->m_Albedo->GetID();

                    if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image1"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec2(2, 2), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
                    {
                        std::string texture = FileDialog::OpenFile("*.png | *.jpg");
                        if (texture != "" && texture != m_SelectedMaterial->m_Albedo->GetPath())
                            m_SelectedMaterial->m_Albedo = TextureManager::Get()->GetTexture(texture);
                    }

                    ImGui::SameLine();
                    bool isAlbedo = m_SelectedMaterial->data.u_HasAlbedo == 1;
                    ImGui::Checkbox("Use##1", &isAlbedo);
                    m_SelectedMaterial->data.u_HasAlbedo = (int)isAlbedo;

                    ImGui::SameLine();
                    ImGui::ColorPicker3("Color", &m_SelectedMaterial->data.m_AlbedoColor.r);
                }
                if (ImGui::CollapsingHeader("AO", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    unsigned int textureID = 0;
                    if (m_SelectedMaterial->HasAO())
                        textureID = m_SelectedMaterial->m_AO->GetID();
                    if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image2"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec2(2, 2), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1)))
                    {
                        std::string texture = FileDialog::OpenFile("Image files (*.png) | *.png | Image files (*.jpg) | *.jpg");
                        if (texture != "")
                        {
                            m_SelectedMaterial->SetAO(TextureManager::Get()->GetTexture(texture));
                        }
                    }
                    ImGui::SameLine();
                    bool hasAO = m_SelectedMaterial->data.u_HasAO == 1;
                    ImGui::Checkbox("Use##1", &hasAO);
                    m_SelectedMaterial->data.u_HasAO = (int)hasAO;

                    ImGui::SameLine();
                    ImGui::DragFloat("Value##2", &m_SelectedMaterial->data.u_AOValue, 0.01f, 0.0f, 1.0f);
                }
                if (ImGui::CollapsingHeader("Normal", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    unsigned int textureID = 0;
                    if (m_SelectedMaterial->HasNormal())
                        textureID = m_SelectedMaterial->m_Normal->GetID();
                    if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image3"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec2(2, 2), ImVec4(0, 0, 0, 1), ImVec4(1, 1, 1, 1)))
                    {
                        std::string texture = FileDialog::OpenFile("*.png");
                        if (texture != "")
                        {
                            m_SelectedMaterial->SetNormal(TextureManager::Get()->GetTexture(texture));
                        }
                    }
                    //ImGui::SameLine();
                    //ImGui::Checkbox("Use##3", &m_SelectedMaterial->data.u_HasNormal);
                }
                if (ImGui::CollapsingHeader("Metalness", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    unsigned int textureID = 0;
                    if (m_SelectedMaterial->HasMetalness())
                        textureID = m_SelectedMaterial->m_Metalness->GetID();
                    if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image4"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec2(2, 2), ImVec4(0, 0, 0, 1), ImVec4(1, 1, 1, 1)))
                    {
                        std::string texture = FileDialog::OpenFile("*.png | *.jpg");
                    }
                    ImGui::SameLine();
                    //ImGui::Checkbox("Use##4", &m_SelectedMaterial->data.u_HasMetalness);
                    ImGui::SameLine();
                    ImGui::DragFloat("Value##4", &m_SelectedMaterial->data.u_MetalnessValue, 0.01f, 0.0f, 1.0f);
                }
                if (ImGui::CollapsingHeader("Roughness", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    unsigned int textureID = 0;
                    if (m_SelectedMaterial->HasRoughness())
                        textureID = m_SelectedMaterial->m_Roughness->GetID();
                    if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image5"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec2(2, 2), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
                    {
                        std::string texture = FileDialog::OpenFile("*.png | *.jpg");
                    }
                    ImGui::SameLine();
                    //ImGui::Checkbox("Use##5", &m_SelectedMaterial->data.u_HasRoughness);
                    ImGui::SameLine();
                    ImGui::DragFloat("Value##5", &m_SelectedMaterial->data.u_RoughnessValue, 0.01f, 0.0f, 1.0f);
                }
            }
            else
            {
                ImGui::Text("No material selected.");
            }


        }
        ImGui::End();
    }

    void NewProject()
    {
        if (Engine::GetProject())
            Engine::GetProject()->Save();

        std::string selectedProject = FileDialog::SaveFile("Project file\0*.project");
        if (selectedProject == "") // Hit cancel
            return;

        Ref<Project> project = Project::New("Unnamed project", "no description", selectedProject + ".project");
        Engine::LoadProject(project);
        Engine::LoadScene(Scene::New());
    }


    ProjectInterface pInterface;
    void OpenProject()
    {
        // Parse the project and load it.
        std::string projectPath = FileDialog::OpenFile("Project file\0*.project");

        if (projectPath == "") // Hit cancel.
            return;

        FileSystem::SetRootDirectory(projectPath + "/../");
        Ref<Project> project = Project::New();
        if (!project->Deserialize(FileSystem::ReadFile(projectPath, true)))
        {
            Logger::Log("Error loading project: " + projectPath, CRITICAL);
            return;
        }

        project->FullPath = projectPath;
        Engine::LoadProject(project);

        pInterface.m_CurrentProject = project;

        Engine::GetCurrentWindow()->SetTitle("Nuake Engine - Editing " + project->Name); 
    }

    void OpenScene()
    {
        // Parse the project and load it.
        std::string projectPath = FileDialog::OpenFile(".scene");

        Ref<Scene> scene = Scene::New();
        if (!scene->Deserialize(FileSystem::ReadFile(projectPath, true))) 
        {
            Logger::Log("Error failed loading scene: " + projectPath, CRITICAL);
            return;
        }

        scene->Path = FileSystem::AbsoluteToRelative(projectPath);
        Engine::LoadScene(scene);
    }

    void EditorInterface::DrawInit()
    {

    }

    json SceneSnapshot;
    void EditorInterface::Draw()
    {
        Init();
        auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;

        if (ImGui::BeginPopupModal("Welcome", NULL, flags))
        {
            ImGui::Text("Welcome to Nuake Engine");
            ImGui::Text("Developement build");

            ImGui::Text("This project is still very early in developement!");
            if (ImGui::Button("New Project"))
                NewProject();

            ImGui::SameLine();
            if (ImGui::Button("Open Project")) 
            {
                OpenProject();
                filesystem.m_CurrentDirectory = FileSystem::RootDirectory;
            }


            ImGui::EndPopup();
        }

        if (!Engine::GetProject())
        {
            ImGui::OpenPopup("Welcome");
            return;
        }

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New project", "CTRL+N"))
                {
                    NewProject();

                    m_IsEntitySelected = false;
                }
                if (ImGui::MenuItem("Open...", "CTRL+O"))
                {
                    OpenProject();

                    m_IsEntitySelected = false;
                }
                if (ImGui::MenuItem("Save", "CTRL+S"))
                {
                    Engine::GetProject()->Save();
                    Engine::GetCurrentScene()->Save();

                    m_IsEntitySelected = false;
                }
                if (ImGui::MenuItem("Save as...", "CTRL+SHIFT+S"))
                {
                    std::string savePath = FileDialog::SaveFile("*.project");
                    Engine::GetProject()->SaveAs(savePath);

                    m_IsEntitySelected = false;
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Set current scene as default")) 
                {
                    Engine::GetProject()->DefaultScene = Engine::GetCurrentScene();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("New scene"))
                {
                    Engine::LoadScene(Scene::New());
                    m_IsEntitySelected = false;
                }
                if (ImGui::MenuItem("Open scene...", "CTRL+SHIFT+O"))
                {
                    OpenScene();
                    m_IsEntitySelected = false;
                }
                if (ImGui::MenuItem("Save scene", "CTR+SHIFT+L+S"))
                {
                    Engine::GetCurrentScene()->Save();
                    m_IsEntitySelected = false;
                }
                if (ImGui::MenuItem("Save scene as...", "CTRL+SHIFT+S"))
                {
                    std::string savePath = FileDialog::SaveFile("*.scene");
                    Engine::GetCurrentScene()->SaveAs(savePath);

                    m_IsEntitySelected = false;
                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
                if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "CTRL+X")) {}
                if (ImGui::MenuItem("Copy", "CTRL+C")) {}
                if (ImGui::MenuItem("Paste", "CTRL+V")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::MenuItem("Lighting", NULL, true)) {}
                if (ImGui::MenuItem("Draw grid", NULL, m_DrawGrid))
                    m_DrawGrid = !m_DrawGrid;
                if (ImGui::MenuItem("Draw collisions", NULL, m_DebugCollisions))
                {
                    m_DebugCollisions = !m_DebugCollisions;
                    PhysicsManager::Get()->SetDrawDebug(m_DebugCollisions);
                }

                if (ImGui::MenuItem("Settings", NULL)) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Entity"))
            {
                if (ImGui::BeginMenu("Create new"))
                {
                    if (ImGui::MenuItem("Empty")) 
                    {
                        auto ent = Engine::GetCurrentScene()->CreateEntity("Empty entity");
                    }
                    if (ImGui::MenuItem("Light")) 
                    {
                        auto ent = Engine::GetCurrentScene()->CreateEntity("Light");
                        ent.AddComponent<LightComponent>();
                    }
                    if (ImGui::MenuItem("Camera")) 
                    {
                        auto ent = Engine::GetCurrentScene()->CreateEntity("Camera");
                        ent.AddComponent<CameraComponent>();
                    }
                    if (ImGui::MenuItem("Rigidbody"))
                    {
                        auto ent = Engine::GetCurrentScene()->CreateEntity("Rigidbody");
                        ent.AddComponent<RigidBodyComponent>();
                    }
                    if (ImGui::MenuItem("Trenchbroom map"))
                    {
                        auto ent = Engine::GetCurrentScene()->CreateEntity("Trenchbroom map");
                        ent.AddComponent<QuakeMapComponent>();
                    }
                    if (ImGui::MenuItem("Mesh")) 
                    {
                        auto ent = Engine::GetCurrentScene()->CreateEntity("Mesh");
                        ent.AddComponent<MeshComponent>();
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::MenuItem("Delete selected", NULL));

                if (ImGui::MenuItem("Duplicate selected", NULL)) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Debug"))
            {
                if (ImGui::MenuItem("Show ImGui demo", NULL, m_ShowImGuiDemo)) m_ShowImGuiDemo = !m_ShowImGuiDemo;
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Quit")) ImGui::EndMenu();
            ImGui::EndMainMenuBar();
        }

        DrawGizmos();

        DrawRessourceWindow();
		
		pInterface.DrawEntitySettings();
        DrawViewport();
        DrawSceneTree();
        //DrawDirectoryExplorer();
        DrawEntityPropreties();
        DrawLogger();

        // new stuff
        filesystem.Draw();
        filesystem.DrawDirectoryExplorer();

        if (m_ShowImGuiDemo)
            ImGui::ShowDemoWindow();

        if (ImGui::Begin("Toolbar", 0, ImGuiWindowFlags_NoScrollbar | ImGuiDockNodeFlags_AutoHideTabBar | ImGuiWindowFlags_NoDecoration))
        {
            if (ImGui::Button(ICON_FA_HAND_POINTER)) CurrentOperation = ImGuizmo::OPERATION::TRANSLATE;
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ARROWS_ALT)) CurrentOperation = ImGuizmo::OPERATION::TRANSLATE;
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_SYNC_ALT)) CurrentOperation = ImGuizmo::OPERATION::ROTATE;
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_EXPAND_ALT)) CurrentOperation = ImGuizmo::OPERATION::SCALE;
            ImGui::SameLine();

            float availWidth = ImGui::GetContentRegionAvailWidth();
            float windowWidth = ImGui::GetWindowWidth();

            float used = windowWidth - availWidth;
            float half = windowWidth / 2.0;
            float needed = half - used;
            ImGui::Dummy(ImVec2(needed, 10));
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_PLAY))
            {
                SceneSnapshot = Engine::GetCurrentScene()->Serialize();
                Engine::EnterPlayMode();
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_STOP) || Input::IsKeyPressed(297))
            {
                Engine::ExitPlayMode();

                Engine::GetCurrentScene()->Deserialize(SceneSnapshot.dump());
                m_IsEntitySelected = false;
            }
        }
        ImGui::End();
    }


    void EditorInterface::BuildFonts()
    {
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
        ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
        normalFont = io.Fonts->AddFontFromFileTTF("resources/Fonts/fa-solid-900.ttf", 11.0f, &icons_config, icons_ranges);
        boldFont = io.Fonts->AddFontFromFileTTF("resources/Fonts/OpenSans-Bold.ttf", 16.0);
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));
        ImGui::GetIO().Fonts->AddFontDefault();
        icons_config.MergeMode = true;
        bigIconFont = io.Fonts->AddFontFromFileTTF("resources/Fonts/fa-solid-900.ttf", 42.0f, &icons_config, icons_ranges);
    }
}

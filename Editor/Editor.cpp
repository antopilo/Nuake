#include <Engine.h>
#include <src/Scene/Entities/Entity.h>

#include <src/Scene/Scene.h>
#include <src/Scene/Components/QuakeMap.h>
#include <src/Vendors/imgui/imgui.h>
#include <src/Vendors/imgui/ImGuizmo.h>
#include <src/Scene/Components/Components.h>
#include <src/Core/Physics/PhysicsManager.h>
#include "../Scene/Components/BoxCollider.h"
#include "src/EditorInterface.h"
#include "../Core/Input.h"
#include <GLFW/glfw3.h>
#include <src/Vendors/glm/trigonometric.hpp>
#include <src/Scripting/ScriptingEngine.h>
#include <src/Resource/FGD/FGDFile.h>


int main()
{
    //std::string TrenchbroomPath = "F:/TrenchBroom/";
    //
    //FGDFile file(TrenchbroomPath + "Games/Nuake/Nuake.fgd");
    //
    //FGDClass newClass(FGDClassType::Point, "light", "a nuake light");
    //
    //ClassProperty prop{
    //    "Intensity",
    //    ClassPropertyType::Integer,
    //    "Changes the light intensity"
    //};
    //
    //newClass.AddProperty(prop);
    //file.AddClass(newClass);
    //
    //file.Save();

    Engine::Init();

    EditorInterface editor;
    editor.BuildFonts();

    Ref<FrameBuffer> myFramebuffer = CreateRef<FrameBuffer>(true, Vector2(200, 200));
    myFramebuffer->SetTexture(CreateRef<Texture>(Vector2(200, 200), GL_RGB));

    Ref<Scene> myScene = CreateRef<Scene>();

    while (!Engine::GetCurrentWindow()->ShouldClose())
    {
        Engine::Tick();

        Engine::Draw();

        if (Input::IsKeyPressed(GLFW_KEY_F8))
            Engine::ExitPlayMode();

        myFramebuffer->Bind();
        myScene->EditorDraw();
        
        myFramebuffer->Unbind();
        if (ImGui::Begin("hahahah"))
        {
            ImVec2 regionAvail = ImGui::GetContentRegionAvail();
            glm::vec2 viewportPanelSize = glm::vec2(regionAvail.x, regionAvail.y);

            if (myFramebuffer->GetSize() != viewportPanelSize)
                myFramebuffer->QueueResize(viewportPanelSize);

            Ref<Texture> texture = myFramebuffer->GetTexture();
            ImGui::Image((void*)texture->GetID(), regionAvail, ImVec2(0, 1), ImVec2(1, 0));
            
        }
        ImGui::End();
        editor.Draw();
        
        Engine::EndDraw();
    }

    Engine::Close();
}


#include <Engine.h>
#include <src/Scene/Entities/Entity.h>

#include <src/Scene/Scene.h>
#include <src/Scene/Entities/Components/QuakeMap.h>
#include <src/Vendors/imgui/imgui.h>
#include <src/Vendors/imgui/ImGuizmo.h>
#include <src/Scene/Entities/Components.h>
#include <src/Core/Physics/PhysicsManager.h>
#include "../Scene/Entities/Components/BoxCollider.h"
#include "src/EditorInterface.h"
#include "../Core/Input.h"
#include <GLFW/glfw3.h>
#include <src/Vendors/glm/trigonometric.hpp>


class FPSCamScript : public ScriptableEntity
{
public:
    float Pitch = 0.0f;
    float Yaw = 0.0f;
    float Speed = 0.0f;
    float mouseLastX = 0.0f;
    float mouseLastY = 0.0f;

    void OnCreate() 
    {
        Input::HideMouse();
    }

    void OnDestroy() 
    {

    }

    void OnUpdate(Timestep ts) 
    {
        float x = Input::GetMouseX();
        float y = Input::GetMouseY();

        // mouse
        float diffx = x - mouseLastX;
        float diffy = mouseLastY - y;
        mouseLastX = x;
        mouseLastY = y;

        const float sensitivity = 0.1f;
        diffx *= sensitivity;
        diffy *= sensitivity;

        Yaw += diffx;
        Pitch += diffy;

        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;

        Ref<Camera> cam = GetComponent<CameraComponent>().CameraInstance;
        cam->cameraDirection.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        cam->cameraDirection.y = sin(glm::radians(Pitch));
        cam->cameraDirection.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        cam->cameraFront = glm::normalize(cam->cameraDirection);
        cam->cameraRight = glm::normalize(glm::cross(cam->up, cam->cameraFront));
    }
};

class MoveScript : public ScriptableEntity
{
public:

    glm::vec3 velocity = glm::vec3(0.0f);
    float playerHeight = 0.75f;
    Ref<Camera> cam;

    void OnCreate()
    {
        Entity ent = Engine::GetCurrentScene()->GetEntity("camEntity");
        cam = ent.GetComponent<CameraComponent>().CameraInstance;
        ent.GetComponent<TransformComponent>().Translation.y = playerHeight;

    }

    void OnDestroy() { }
    
    void OnUpdate(Timestep ts)
    {
        if (Input::IsKeyPress(GLFW_KEY_W))
            velocity.z = 5.0f;
        else if (Input::IsKeyPress(GLFW_KEY_S))
            velocity.z = -5.0f;
        else
            velocity.z = 0.0f;
        if (Input::IsKeyPress(GLFW_KEY_A))
            velocity.x = 5.0f;
        else if (Input::IsKeyPress(GLFW_KEY_D))
            velocity.x = -5.0f;
        else
            velocity.x = 0.0f;

       glm::vec3 front = cam->cameraFront;
       front.y = .0f;

       glm::vec3 right = cam->cameraRight;
       right.y = .0f;
       glm::vec3 result = velocity.z * glm::normalize(front) + velocity.x * normalize(right);

       CharacterControllerComponent& ccc = GetComponent<CharacterControllerComponent>();

       if (ccc.CharacterController->IsOnGround && Input::IsKeyPress(GLFW_KEY_SPACE))
       {
           velocity.y = 100.0f;
       }
           
       ccc.CharacterController->MoveAndSlide(result);
    }
};

class FlashingLightScript : public ScriptableEntity
{
public:
    bool isOn = false;
    float nextFlip = 0.f;
    float strength = 10.f;

    float deltaTime = 0.f;

    void OnCreate() { }

    void OnDestroy() { }

    void OnUpdate(Timestep ts)
    {
        if (nextFlip < deltaTime)
        {
            GetComponent<LightComponent>().Strength = isOn * strength;
            isOn = !isOn;
            nextFlip = deltaTime + 1.0f;
        }

        deltaTime += ts;
    }
};

void CreateScene()
{
    Ref<Scene> scene = Engine::GetCurrentScene();

    // Main map entity
    auto mapEntity = scene->CreateEntity("Map");
    mapEntity.AddComponent<QuakeMap>();
    mapEntity.GetComponent<QuakeMap>().Load("Res/Maps/texture_test.map", true);
    mapEntity.GetComponent<QuakeMap>().HasCollisions = true;

    // Light entity
    auto lightEntity = scene->CreateEntity("Light");
    lightEntity.AddComponent<LightComponent>();
    lightEntity.GetComponent<LightComponent>().SyncDirectionWithSky = true;
    lightEntity.GetComponent<LightComponent>().Type = Directional;
    // lightEntity.AddComponent<NativeScriptComponent>().Bind<FlashingLightScript>();

    // Create player
    auto player = scene->CreateEntity("Player");
    player.GetComponent<TransformComponent>().Translation.y = 10.f;
    player.AddComponent<CharacterControllerComponent>();
    player.AddComponent<NativeScriptComponent>().Bind<MoveScript>();

    // Add cam child
    auto childEntity = scene->CreateEntity("camEntity");
    childEntity.AddComponent<CameraComponent>();
    childEntity.AddComponent<NativeScriptComponent>().Bind<FPSCamScript>();

    // cam as child of player.
    player.AddChild(childEntity);
}

int main()
{
    Engine::Init();

    EditorInterface editor;
    editor.BuildFonts();

    CreateScene();

    while (!Engine::GetCurrentWindow()->ShouldClose())
    {
        Engine::Draw();

        if (Input::IsKeyPress(GLFW_KEY_F8))
            Engine::ExitPlayMode();

        editor.Draw();

        Engine::EndDraw();
        Engine::Tick();
    }

    Engine::Close();
}


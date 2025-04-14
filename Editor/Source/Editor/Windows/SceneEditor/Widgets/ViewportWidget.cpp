#include "ViewportWidget.h"

#include <Nuake/UI/ImUI.h>
#include "Nuake/Core/Input.h"

#include "../../EditorInterface.h"

#include "Nuake/Rendering/Vulkan/VulkanRenderer.h"
#include "Nuake/Rendering/Vulkan/SceneViewport.h"
#include "Nuake/Rendering/Vulkan/DebugCmd.h"

#include <glm/gtc/type_ptr.hpp>
#include <Nuake/Scene/Components/AudioEmitterComponent.h>
#include <Nuake/Scene/Components/ParticleEmitterComponent.h>
#include <Nuake/Scene/Components/RigidbodyComponent.h>

#include <Nuake/Scene/Components/BoxCollider.h>
#include <Nuake/Scene/Components/SphereCollider.h>

using namespace Nuake;

ViewportWidget::ViewportWidget(EditorContext& context) : IEditorWidget(context)
{
    OnSceneChanged(context.GetScene());
}

ViewportWidget::~ViewportWidget()
{
    VkRenderer::Get().RemoveViewport(sceneViewport->GetID());
}

void ViewportWidget::Update(float ts)
{
	editorContext.GetScene()->Update(ts);

    if (!Engine::IsPlayMode())
    {
        EditorCamera& editorCam = reinterpret_cast<EditorCamera&>(*editorContext.GetScene()->GetCurrentCamera().get());
        IsControllingCamera = editorCam.Update(ts, isHoveringViewport);
    }

    const Vector2 viewportSize = sceneViewport->GetViewportSize();
    editorContext.GetScene()->GetCurrentCamera()->OnWindowResize(viewportSize.x, viewportSize.y);

	if (editorContext.GetSelection().Type == EditorSelectionType::Entity)
	{
		sceneViewport->SetSelectedEntityID(editorContext.GetSelection().Entity.GetHandle());
	}
    else
    {
        sceneViewport->SetSelectedEntityID(-1);
    }
}

void ViewportWidget::Draw()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	if (BeginWidgetWindow(ICON_FA_GAMEPAD + std::string("Viewport")))
	{
		ImGui::PopStyleVar();

        DrawOverlay();

		ImGuizmo::BeginFrame();
		ImGuizmo::SetOrthographic(false);

		ImVec2 regionAvail = ImGui::GetContentRegionAvail();
		Vector2 viewportPanelSize = glm::vec2(regionAvail.x, regionAvail.y);

        bool needsResize = sceneViewport->QueueResize(viewportPanelSize);
        editorContext.GetScene()->GetCurrentCamera()->OnWindowResize(regionAvail.x, regionAvail.y);

		// This is important for make UI mouse coord relative to viewport
		// Nuake::Input::SetViewportDimensions(m_ViewportPos, viewportPanelSize);

		VkDescriptorSet textureDesc = sceneViewport->GetRenderTarget()->GetImGuiDescriptorSet();

		ImVec2 imagePos = ImGui::GetWindowPos() + ImGui::GetCursorPos();
        ImVec2 viewportMin = ImGui::GetCursorScreenPos();
		// Input::SetEditorViewportSize(m_ViewportPos, viewportPanelSize);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		//m_ViewportPos = { imagePos.x, imagePos.y };
		ImGui::Image(textureDesc, regionAvail, { 0, 1 }, { 1, 0 });
		ImGui::PopStyleVar();

		const Vector2& mousePos = Input::GetMousePosition();

		const ImVec2& windowPos = ImGui::GetWindowPos();
		const auto windowPosNuake = Vector2(windowPos.x, windowPos.y);
		const ImVec2& windowSize = ImGui::GetWindowSize();
		const bool isInsideWidth = mousePos.x > windowPos.x && mousePos.x < windowPos.x + windowSize.x;
		const bool isInsideHeight = mousePos.y > windowPos.y && mousePos.y < windowPos.y + windowSize.y;
		this->isHoveringViewport = isInsideWidth && isInsideHeight;

		// TODO(antopilo) drag n drop
		ImGuizmo::SetDrawlist();
		ImGuizmo::AllowAxisFlip(true);
		ImGuizmo::SetRect(viewportMin.x, viewportMin.y, regionAvail.x, regionAvail.y);

		// TODO(grid)
        auto selection = editorContext.GetSelection();
        if (selection.Type == EditorSelectionType::Entity && !Engine::IsPlayMode())
        {
            if (!selection.Entity.IsValid())
            {
				editorContext.SetSelection(EditorSelection());
            }
            else
            {
                TransformComponent& tc = selection.Entity.GetComponent<TransformComponent>();
                Matrix4 transform = tc.GetGlobalTransform();
                auto editorCam = editorContext.GetScene()->GetCurrentCamera();
                Matrix4 cameraView = editorCam->GetTransform();

                // Since imguizmo doesnt support reverse-Z, we need to create a new projection matrix
                // With a normal near and far plane.
                Matrix4 normalZProjection = glm::perspectiveFov(glm::radians(editorCam->Fov), 9.0f * editorCam->AspectRatio, 9.0f, editorCam->Far, editorCam->Near);
                static Vector3 camPreviousPos = editorContext.GetScene()->m_EditorCamera->Translation;
                static Vector3 camNewPos = Vector3(0, 0, 0);
                Vector3 camDelta = camNewPos - camPreviousPos;
                Vector3 previousGlobalPos = transform[3];
                // Imguizmo calculates the delta from the gizmo,
                ImGuizmo::Manipulate(
                    glm::value_ptr(editorCam->GetTransform()),
                    glm::value_ptr(normalZProjection),
                    CurrentOperation, CurrentMode,
                    glm::value_ptr(transform), NULL,
                    UseSnapping ? &CurrentSnapping.x : NULL
                );

                if (ImGuizmo::IsUsing())
                {
                    // Since imguizmo returns a transform in global space and we want the local transform,
                    // we need to multiply by the inverse of the parent's global transform in order to revert
                    // the changes from the parent transform.
                    Matrix4 localTransform = Matrix4(transform);

                    Vector3 newGlobalPos = transform[3];
                    if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
                    {
                        Vector3 positionDelta = newGlobalPos - previousGlobalPos;
                        editorContext.GetScene()->m_EditorCamera->Translation += positionDelta;
                        camNewPos = editorContext.GetScene()->m_EditorCamera->Translation;
                    }

                    ParentComponent& parent = selection.Entity.GetComponent<ParentComponent>();
                    if (parent.HasParent)
                    {
                        const auto& parentTransformComponent = parent.Parent.GetComponent<TransformComponent>();
                        const Matrix4& parentTransform = parentTransformComponent.GetGlobalTransform();
                        localTransform = glm::inverse(parentTransform) * localTransform;
                    }

                    // Decompose local transform
                    float decomposedPosition[3];
                    float decomposedEuler[3];
                    float decomposedScale[3];
                    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(localTransform), decomposedPosition, decomposedEuler, decomposedScale);

                    const auto& localPosition = Vector3(decomposedPosition[0], decomposedPosition[1], decomposedPosition[2]);
                    const auto& localScale = Vector3(decomposedScale[0], decomposedScale[1], decomposedScale[2]);

                    localTransform[0] /= localScale.x;
                    localTransform[1] /= localScale.y;
                    localTransform[2] /= localScale.z;
                    const auto& rotationMatrix = Matrix3(localTransform);
                    const Quat& localRotation = glm::normalize(Quat(rotationMatrix));

                    const Matrix4& rotationMatrix4 = glm::mat4_cast(localRotation);
                    const Matrix4& scaleMatrix = glm::scale(Matrix4(1.0f), localScale);
                    const Matrix4& translationMatrix = glm::translate(Matrix4(1.0f), localPosition);
                    const Matrix4& newLocalTransform = translationMatrix * rotationMatrix4 * scaleMatrix;

                    tc.Translation = localPosition;

                    if (CurrentOperation != ImGuizmo::SCALE)
                    {
                        tc.Rotation = localRotation;
                    }

                    tc.Scale = localScale;
                    tc.LocalTransform = newLocalTransform;
                    tc.Dirty = true;
                }
            }
        }

	}
	else
	{
		ImGui::PopStyleVar();
	}
	ImGui::End();
}

void ViewportWidget::OnSceneChanged(Ref<Nuake::Scene> scene)
{
    auto& vkRenderer = Nuake::VkRenderer::Get();

    // Recreate new viewport with new scene with the same resolution
    Vector2 currentResolution = Vector2{ 1, 1 };

    if (sceneViewport)
    {
        currentResolution = sceneViewport->GetViewportSize();

        // Remove old viewport
        vkRenderer.RemoveViewport(sceneViewport->GetID());
    }

    // Create new viewport with same reoslution
    const UUID viewId = editorContext.GetScene()->GetCurrentCamera()->ID;
    auto viewport = vkRenderer.CreateViewport(viewId, currentResolution);
    vkRenderer.RegisterSceneViewport(scene, viewport->GetID());

    sceneViewport = viewport;

	sceneViewport->GetOnDebugDraw().AddRaw(this, &ViewportWidget::OnDebugDraw);
    sceneViewport->GetOnLineDraw().AddRaw(this, &ViewportWidget::OnLineDraw);
}

float GetGizmoScale(const Vector3& camPosition, const Nuake::Vector3& position)
{
    float distance = Distance(camPosition, position);

    constexpr float ClosestDistance = 3.5f;
    if (distance < ClosestDistance)
    {
        float fraction = distance / ClosestDistance;
        return fraction;
    }

    return 1.0f;
}

template<typename T>
void DrawIconGizmo(DebugCmd& debugCmd, const std::string& icon)
{
    auto scene = debugCmd.GetScene();
    auto cam = scene->GetCurrentCamera();
    auto view = cam->GetTransform();
    auto proj = cam->GetPerspective();
    const Vector3& cameraPosition = cam->GetTranslation();
    const Vector3 gizmoSize = Vector3(Engine::GetProject()->Settings.GizmoSize);
    auto camView = scene->m_Registry.view<TransformComponent, T>();
    for (auto e : camView)
    {
        auto [transform, cam] = scene->m_Registry.get<TransformComponent, T>(e);

        Matrix4 initialTransform = transform.GetGlobalTransform();
        Matrix4 gizmoTransform = initialTransform;
        gizmoTransform = glm::inverse(scene->GetCurrentCamera()->GetTransform());
        gizmoTransform[3] = initialTransform[3];
        gizmoTransform = glm::scale(gizmoTransform, gizmoSize * GetGizmoScale(cameraPosition, initialTransform[3]));
        debugCmd.DrawTexturedQuad(proj * view * gizmoTransform, TextureManager::Get()->GetTexture2(icon));
    }
}

void ViewportWidget::OnLineDraw(DebugLineCmd& lineCmd)
{
    auto scene = lineCmd.GetScene();
    auto cam = scene->GetCurrentCamera();
    const Vector3& cameraPosition = cam->GetTranslation();

    auto view = cam->GetTransform();
    auto proj = cam->GetPerspective();

    Matrix4 transform = Matrix4(1.0f);
    transform = glm::translate(transform, { 0, 2, 0 });
    //lineCmd.DrawLine(proj * view * transform, Color(1, 0, 0, 1), 2.0f);

    auto camView = scene->m_Registry.view<TransformComponent, CameraComponent>();
    for (auto e : camView)
    {
        auto [transform, camera] = scene->m_Registry.get<TransformComponent, CameraComponent>(e);
        const Quat& globalRotation = glm::normalize(transform.GetGlobalRotation());
        const Matrix4& rotationMatrix = glm::mat4_cast(globalRotation);

        const float aspectRatio = camera.CameraInstance->AspectRatio;
        const float fov = camera.CameraInstance->Fov;

        Matrix4 clampedProj = glm::perspectiveFov(glm::radians(fov), 9.0f * aspectRatio, 9.0f, 0.05f, 3.0f);
        Matrix4 boxTransform = glm::translate(scene->m_EditorCamera->GetTransform(), Vector3(transform.GetGlobalTransform()[3])) * rotationMatrix * glm::inverse(clampedProj);
        lineCmd.DrawBox(proj * boxTransform, Color(1, 0, 0, 1.0f), 1.5f, false);
    }

    auto boxColliderView = scene->m_Registry.view<TransformComponent, BoxColliderComponent>();
    for (auto e : boxColliderView)
    {
        auto [transform, boxCollider] = scene->m_Registry.get<TransformComponent, BoxColliderComponent>(e);

        Vector3 boxSize = boxCollider.GetSize();
        Matrix4 boxTransform = Matrix4(1.0f); 
		boxTransform = glm::translate(boxTransform, Vector3(transform.GetGlobalTransform()[3]));
		boxTransform = glm::scale(boxTransform, boxSize);
        lineCmd.DrawBox(proj * view * boxTransform, Color(0, 1, 0, 1.0f), 1.5f, boxCollider.IsTrigger);
    }

    auto sphereColliderView = scene->m_Registry.view<TransformComponent, SphereColliderComponent>();
    for (auto e : sphereColliderView)
    {
        auto [transformComponent, sphereCollider] = scene->m_Registry.get<TransformComponent, SphereColliderComponent>(e);

        float radius = sphereCollider.GetRadius();
        Matrix4 transform = Matrix4(1.0f);
        transform = glm::translate(transform, Vector3(transformComponent.GetGlobalTransform()[3]));
        transform = glm::scale(transform, Vector3(sphereCollider.GetRadius()));
        lineCmd.DrawSphere(proj * view * transform, Color(1, 0, 0, 1.0f), 1.5f, true);
    }

    auto capsuleColliderView = scene->m_Registry.view<TransformComponent, CapsuleColliderComponent>();
    for (auto e : capsuleColliderView)
    {
        auto [transformComponent, capsuleCollider] = scene->m_Registry.get<TransformComponent, CapsuleColliderComponent>(e);

        Matrix4 transform = Matrix4(1.0f);
        transform = glm::translate(transform, Vector3(transformComponent.GetGlobalTransform()[3]));
        transform = glm::scale(transform, Vector3(capsuleCollider.Radius, capsuleCollider.Height, capsuleCollider.Radius));
        lineCmd.DrawCapsule(proj * view * transform, Color(1, 0, 0, 1.0f), 1.5f, true);
    }

    auto cylinderColliderView = scene->m_Registry.view<TransformComponent, CylinderColliderComponent>();
    for (auto e : cylinderColliderView)
    {
        auto [transformComponent, cylinderCollider] = scene->m_Registry.get<TransformComponent, CylinderColliderComponent>(e);

        Matrix4 transform = Matrix4(1.0f);
        transform = glm::translate(transform, Vector3(transformComponent.GetGlobalTransform()[3]));
        transform = glm::scale(transform, Vector3(cylinderCollider.Radius, cylinderCollider.Height, cylinderCollider.Radius));
        lineCmd.DrawCylinder(proj * view * transform, Color(1, 0, 0, 1.0f), 1.5f, true);
    }

    lineCmd.DrawBox(proj * view * transform, { 0, 1, 0, 1 }, 1.5f, true);
    //lineCmd.DrawArrow({3, 3, 0}, {7, 3, 0}, proj, view, Color(1, 0, 0, 1), 3.0f);
}

void ViewportWidget::OnDebugDraw(DebugCmd& debugCmd)
{
    auto scene = debugCmd.GetScene();
    auto cam = scene->GetCurrentCamera();
    const Vector3& cameraPosition = cam->GetTranslation();
    const Vector3 gizmoSize = Vector3(Engine::GetProject()->Settings.GizmoSize);

    auto view = cam->GetTransform();
    auto proj = cam->GetPerspective();

    static auto drawGizmoIcon = [&](TransformComponent& transform, const std::string& icon)
    {
        Matrix4 initialTransform = transform.GetGlobalTransform();
        Matrix4 gizmoTransform = initialTransform;
        gizmoTransform = glm::inverse(scene->GetCurrentCamera()->GetTransform());
        gizmoTransform[3] = initialTransform[3];
        gizmoTransform = glm::scale(gizmoTransform, gizmoSize * GetGizmoScale(cameraPosition, initialTransform[3]));
        debugCmd.DrawTexturedQuad(proj * view * gizmoTransform, TextureManager::Get()->GetTexture2(icon));
    };
    
	debugCmd.DrawQuad(proj * view * glm::translate(Matrix4(1.0f), { 0, 4, 0 }));

    auto lightView = scene->m_Registry.view<TransformComponent, LightComponent>();
    for (auto e : lightView)
    {
        auto [transform, light] = scene->m_Registry.get<TransformComponent, LightComponent>(e);

        std::string texturePath = "Resources/Gizmos/";
        switch (light.Type)
        {
        case LightType::Point:
            texturePath += "light.png";
            break;
        case LightType::Directional:
            texturePath += "light_directional.png";
            break;
        case LightType::Spot:
            texturePath += "light_spot.png";
            break;
        default:
            texturePath += "light.png";
        }

        // Billboard + scaling logic
        drawGizmoIcon(transform, texturePath);
    }

    DrawIconGizmo<CameraComponent>(debugCmd, "Resources/Gizmos/Camera.png");
    DrawIconGizmo<CharacterControllerComponent>(debugCmd, "Resources/Gizmos/player.png");
    DrawIconGizmo<BoneComponent>(debugCmd, "Resources/Gizmos/bone.png");
    DrawIconGizmo<AudioEmitterComponent>(debugCmd, "Resources/Gizmos/sound_emitter.png");
    DrawIconGizmo<RigidBodyComponent>(debugCmd, "Resources/Gizmos/rigidbody.png");
    DrawIconGizmo<ParticleEmitterComponent>(debugCmd, "Resources/Gizmos/particles.png");
}


void ViewportWidget::DrawOverlay()
{
    if (Engine::GetGameState() == GameState::Playing)
    {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    const float DISTANCE = 10.0f;
    int corner = 0;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    window_flags |= ImGuiWindowFlags_NoMove;
    ImGuiViewport* viewport = ImGui::GetWindowViewport();
    float title_bar_height = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2;
    ImVec2 work_area_pos = ImGui::GetCurrentWindow()->Pos + ImVec2(0, title_bar_height);   // Instead of using viewport->Pos we use GetWorkPos() to avoid menu bars, if any!
    ImVec2 work_area_size = ImGui::GetCurrentWindow()->Size;
    ImVec2 window_pos = ImVec2((corner & 1) ? (work_area_pos.x + work_area_size.x - DISTANCE) : (work_area_pos.x + DISTANCE), (corner & 2) ? (work_area_pos.y + work_area_size.y - DISTANCE) : (work_area_pos.y + DISTANCE));
    ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 32.0f);

    bool showOverlay = true;
    if (ImGui::Begin("ActionBar", &showOverlay, window_flags))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100);

        bool selectedMode = CurrentOperation == ImGuizmo::OPERATION::TRANSLATE;
        if (selectedMode)
        {
            Color color = Engine::GetProject()->Settings.PrimaryColor;
            ImGui::PushStyleColor(ImGuiCol_Button, { color.r, color.g, color.b, 1.0f });
        }

        if (ImGui::Button(ICON_FA_ARROWS_ALT, ImVec2(30, 28)) || (ImGui::Shortcut(ImGuiKey_W, 0, ImGuiInputFlags_RouteGlobalLow) && !ImGui::IsAnyItemActive() && !IsControllingCamera))
        {
            CurrentOperation = ImGuizmo::OPERATION::TRANSLATE;
        }


        UI::Tooltip("Translate");
        if (selectedMode)
        {
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();

        selectedMode = CurrentOperation == ImGuizmo::OPERATION::ROTATE;
        if (selectedMode)
        {
            Color color = Engine::GetProject()->Settings.PrimaryColor;
            ImGui::PushStyleColor(ImGuiCol_Button, { color.r, color.g, color.b, 1.0f });
        }

        if (ImGui::Button(ICON_FA_SYNC_ALT, ImVec2(30, 28)) || (ImGui::Shortcut(ImGuiKey_E, 0, ImGuiInputFlags_RouteGlobalLow) && !ImGui::IsAnyItemActive() && !IsControllingCamera))
        {
            CurrentOperation = ImGuizmo::OPERATION::ROTATE;
        }

        UI::Tooltip("Rotate");

        if (selectedMode)
        {
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();

        selectedMode = CurrentOperation == ImGuizmo::OPERATION::SCALE;
        if (selectedMode)
        {
            Color color = Engine::GetProject()->Settings.PrimaryColor;
            ImGui::PushStyleColor(ImGuiCol_Button, { color.r, color.g, color.b, 1.0f });
        }

        if (ImGui::Button(ICON_FA_EXPAND_ALT, ImVec2(30, 28)) || (ImGui::Shortcut(ImGuiKey_R, 0, ImGuiInputFlags_RouteGlobalLow) && !ImGui::IsAnyItemActive() && !IsControllingCamera))
        {
            CurrentOperation = ImGuizmo::OPERATION::SCALE;
        }

        UI::Tooltip("Scale");

        if (selectedMode)
        {
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();

        selectedMode = CurrentMode == ImGuizmo::MODE::WORLD;
        if (selectedMode)
        {
            Color color = Engine::GetProject()->Settings.PrimaryColor;
            ImGui::PushStyleColor(ImGuiCol_Button, { color.r, color.g, color.b, 1.0f });
        }

        if (ImGui::Button(ICON_FA_GLOBE, ImVec2(30, 28)))
        {
            CurrentMode = ImGuizmo::MODE::WORLD;
        }

        UI::Tooltip("Global Transformation");

        if (selectedMode)
        {
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();

        selectedMode = CurrentMode == ImGuizmo::MODE::LOCAL;
        if (selectedMode)
        {
            Color color = Engine::GetProject()->Settings.PrimaryColor;
            ImGui::PushStyleColor(ImGuiCol_Button, { color.r, color.g, color.b, 1.0f });
        }

        if (ImGui::Button(ICON_FA_CUBE, ImVec2(30, 28)))
        {
            CurrentMode = ImGuizmo::MODE::LOCAL;
        }

        UI::Tooltip("Local Transformation");

        if (selectedMode)
        {
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();

        ImGui::SameLine();
        ImGui::PushItemWidth(75);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 6, 6 });
        ImGui::DragFloat("##snapping", &CurrentSnapping.x, 0.01f, 0.0f, 100.0f);
        CurrentSnapping = { CurrentSnapping.x, CurrentSnapping.x, CurrentSnapping.x };
        ImGui::PopStyleVar();

        ImGui::PopItemWidth();
        UI::Tooltip("Snapping");

        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }

    ImGui::PopStyleVar();
    ImGui::End();

    corner = 1;
    window_flags |= ImGuiWindowFlags_NoMove;
    viewport = ImGui::GetWindowViewport();
    work_area_pos = ImGui::GetCurrentWindow()->Pos;   // Instead of using viewport->Pos we use GetWorkPos() to avoid menu bars, if any!
    work_area_size = ImGui::GetCurrentWindow()->Size;
    window_pos = ImVec2((corner & 1) ? (work_area_pos.x + work_area_size.x - DISTANCE) : (work_area_pos.x + DISTANCE), (corner & 2) ? (work_area_pos.y + work_area_size.y - DISTANCE) : (work_area_pos.y + DISTANCE));
    window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::SetNextWindowViewport(viewport->ID);

    int corner2 = 1;
    work_area_pos = ImGui::GetCurrentWindow()->Pos;   // Instead of using viewport->Pos we use GetWorkPos() to avoid menu bars, if any!
    work_area_size = ImGui::GetCurrentWindow()->Size;
    window_pos = ImVec2((corner2 & 1) ? (work_area_pos.x + work_area_size.x - DISTANCE) : (work_area_pos.x + DISTANCE), (corner2 & 2) ? (work_area_pos.y + work_area_size.y - DISTANCE) : (work_area_pos.y + DISTANCE));
    window_pos_pivot = ImVec2((corner2 & 1) ? 1.0f : 0.0f, (corner2 & 2) ? 1.0f : 0.0f);
    ImGui::SetNextWindowPos(window_pos + ImVec2(0, 40), ImGuiCond_Always, window_pos_pivot);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 32.0f);
    ImGui::SetNextWindowSize(ImVec2(16, ImGui::GetContentRegionAvail().y - DISTANCE * 2.0 - 40.0));
    if (IsControllingCamera)
    {
        if (ImGui::Begin("Controls", &showOverlay, window_flags))
        {
            const auto& editorCam = Engine::GetCurrentScene()->m_EditorCamera;
            const float camSpeed = editorCam->Speed;

            const float maxSpeed = 50.0f;
            const float minSpeed = 0.05f;
            const float normalizedSpeed = glm::clamp((camSpeed / maxSpeed), 0.0f, 1.0f);

            ImVec2 start = ImGui::GetWindowPos() - ImVec2(0.0, 4.0);
            ImVec2 end = start + ImGui::GetWindowSize() - ImVec2(0, 16.0);
            ImVec2 startOffset = ImVec2(start.x, end.y - (normalizedSpeed * (ImGui::GetWindowHeight() - 20.0)));

            ImGui::GetWindowDrawList()->AddRectFilled(startOffset + ImVec2(0, 10.0), end + ImVec2(0.0, 20.0), IM_COL32(255, 255, 255, 180), 8.0f, ImDrawFlags_RoundCornersAll);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100);
            ImGui::PopStyleVar();
        }
        ImGui::End();
    }
    ImGui::PopStyleVar();
}
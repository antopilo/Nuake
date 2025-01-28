#include "ViewportWidget.h"

#include <src/UI/ImUI.h>
#include "src/Core/Input.h"

#include "../../EditorInterface.h"

#include "src/Rendering/Vulkan/VulkanRenderer.h"
#include "src/Rendering/Vulkan/SceneViewport.h"

#include <glm/gtc/type_ptr.hpp>

using namespace Nuake;

ViewportWidget::ViewportWidget(EditorContext& context) : IEditorWidget(context)
{
    
	const Vector2& defaultSize = { 1280, 720 };
	const UUID viewId = editorContext.GetScene()->m_EditorCamera->ID;
    auto& vkRenderer = Nuake::VkRenderer::Get();
    auto viewport = vkRenderer.CreateViewport(viewId, defaultSize);
    vkRenderer.RegisterSceneViewport(context.GetScene(), viewport->GetID());
	sceneViewport = viewport;
}

ViewportWidget::~ViewportWidget()
{
    Nuake::VkRenderer::Get().RemoveViewport(sceneViewport->GetID());
}

void ViewportWidget::Update(float ts)
{

}

void ViewportWidget::Draw()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	if (BeginWidgetWindow(ICON_FA_GAMEPAD + std::string("Viewport")))
	{
		ImGui::PopStyleVar();

		ImGuizmo::BeginFrame();
		ImGuizmo::SetOrthographic(false);

		ImVec2 regionAvail = ImGui::GetContentRegionAvail();
		Vector2 viewportPanelSize = glm::vec2(regionAvail.x, regionAvail.y);

		// This is important for make UI mouse coord relative to viewport
		// Input::SetViewportDimensions(m_ViewportPos, viewportPanelSize);

		VkDescriptorSet textureDesc = VkRenderer::Get().DrawImage->GetImGuiDescriptorSet();

		ImVec2 imagePos = ImGui::GetWindowPos() + ImGui::GetCursorPos();
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
		// m_IsHoveringViewport = isInsideWidth && isInsideHeight;

		// TODO(antopilo) drag n drop
		ImGuizmo::SetDrawlist();
		ImGuizmo::AllowAxisFlip(true);
		ImGuizmo::SetRect(imagePos.x, imagePos.y, viewportPanelSize.x, viewportPanelSize.y);

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
                const auto& editorCam = Engine::GetCurrentScene()->GetCurrentCamera();
                Matrix4 cameraView = editorCam->GetTransform();

                // Since imguizmo doesnt support reverse-Z, we need to create a new projection matrix
                // With a normal near and far plane.
                Matrix4 normalZProjection = glm::perspectiveFov(glm::radians(editorCam->Fov), 9.0f * editorCam->AspectRatio, 9.0f, editorCam->Far, editorCam->Near);

                static Vector3 camPreviousPos = Engine::GetCurrentScene()->m_EditorCamera->Translation;
                static Vector3 camNewPos = Vector3(0, 0, 0);
                Vector3 camDelta = camNewPos - camPreviousPos;
                Vector3 previousGlobalPos = transform[3];
                // Imguizmo calculates the delta from the gizmo,
                ImGuizmo::Manipulate(
                    glm::value_ptr(Engine::GetCurrentScene()->GetCurrentCamera()->GetTransform()),
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
                        Engine::GetCurrentScene()->m_EditorCamera->Translation += positionDelta;
                        camNewPos = Engine::GetCurrentScene()->m_EditorCamera->Translation;
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
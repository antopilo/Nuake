#include "PrefabEditorWindow.h"

#include <src/Core/Input.h>
#include <src/Rendering/Buffers/FrameBuffer.h>
#include <src/Rendering/SceneRenderer.h>
#include <src/Resource/Prefab.h>
#include <src/Scene/Scene.h>
#include <src/Scene/EditorCamera.h>
#include <src/Scene/Entities/Entity.h>
#include <src/Rendering/Textures/Texture.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <src/Scene/Components/SpriteComponent.h>
#include <src/Scene/Components/ParticleEmitterComponent.h>
#include <src/Scene/Components/RigidbodyComponent.h>
#include <src/Scene/Components/BoxCollider.h>
#include <src/Scene/Components/SphereCollider.h>
#include <src/Scene/Components/AudioEmitterComponent.h>
#include <src/Scene/Components/PrefabComponent.h>
#include <src/Scene/Components/BSPBrushComponent.h>
#include <src/Scene/Components/NetScriptComponent.h>
#include <src/FileSystem/FileDialog.h>


using namespace Nuake;

PrefabEditorWindow::PrefabEditorWindow(Ref<Prefab> inPrefab) :
	prefab(inPrefab)
{
	const Vector2 defaultSize = Vector2{ 640, 360 };
	viewportFramebuffer = CreateRef<FrameBuffer>(true, defaultSize);
	viewportFramebuffer->SetTexture(CreateRef<Texture>(defaultSize, GL_RGB, GL_RGB16F, GL_FLOAT));

	virtualScene = CreateRef<Scene>();
	virtualScene->GetEnvironment()->CurrentSkyType = SkyType::ProceduralSky;
	virtualScene->GetEnvironment()->ProceduralSkybox->SunDirection = { 0.58f, 0.34f, -0.74f };
	
	Prefab::InstanceInScene(inPrefab->Path, virtualScene);

	Ref<Texture> outputTexture = CreateRef<Texture>(defaultSize, GL_RGB);
	outputTexture->SetParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	outputTexture->SetParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	auto& previewLight = virtualScene->CreateEntity("_directionalLight").AddComponent<LightComponent>();
	previewLight.Type = LightType::Directional;
	previewLight.SetCastShadows(true);
	previewLight.SyncDirectionWithSky = true;
	previewLight.Strength = 5.5f;

}

void PrefabEditorWindow::Update(float ts)
{

}

void PrefabEditorWindow::Draw()
{
	RenderScene();

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

	ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
	ImGuiID dockspace_id = ImGui::GetID(prefab->Path.c_str());
	ImVec2 dockspace_size;
	if (!isInitialized)
	{
		ImGui::SetNextWindowSize(ImVec2(1280, 720));
	}

	if(ImGui::Begin(prefab->Path.c_str(), 0, window_flags))
	{
		dockspace_size = ImGui::GetContentRegionAvail();
		ImGui::DockSpace(dockspace_id, dockspace_size, dockspace_flags);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				ImGui::MenuItem("New", "Ctrl+N");
				ImGui::MenuItem("Open", "Ctrl+O");
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

	}
	ImGui::End();

	// Programmatically dock windows using DockBuilder API
	if (!isInitialized)
	{
		isInitialized = true;

		// Create dock layout for the embedded dockspace
		ImGui::DockBuilderRemoveNode(dockspace_id); // Clear any existing layout
		ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspace_id, dockspace_size);

		// Split the dockspace into two areas: left and right
		ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.25f, nullptr, &dockspace_id);
		ImGuiID dock_r_id = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.66f, nullptr, &dockspace_id);

		// Dock windows into the created nodes
		ImGui::DockBuilderDockWindow(std::string("Prefab Hierarchy## " + prefab->Path).c_str(), dock_left_id);
		ImGui::DockBuilderDockWindow(std::string("Viewport##" + prefab->Path).c_str(), dockspace_id);
		ImGui::DockBuilderDockWindow(std::string("Properties##" + prefab->Path).c_str(), dock_r_id);
		// Commit the dock layout
		ImGui::DockBuilderFinish(dockspace_id);
	}

	if (ImGui::Begin(std::string("Prefab Hierarchy## " + prefab->Path).c_str()))
	{
		Ref<Scene> scene = virtualScene;
		std::string searchQuery = "";

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 8, 8 });
		ImGui::InputTextWithHint("##search", "Search entity", &searchQuery, 0, 0, 0);
		ImGui::PopStyleVar();

		ImGui::SameLine();

		if (UI::PrimaryButton("Add Entity", { ImGui::GetContentRegionAvail().x, 0 }))
		{
			ImGui::OpenPopup("create_entity_popup");
		}

		if (ImGui::BeginPopup("create_entity_popup"))
		{
			Nuake::Entity entity;
			if (ImGui::MenuItem("Empty"))
			{
				entity = scene->CreateEntity("Empty");
			}

			if (ImGui::BeginMenu("3D"))
			{
				if (ImGui::MenuItem("Camera"))
				{
					entity = scene->CreateEntity("Camera");
					entity.AddComponent<CameraComponent>();
				}
				if (ImGui::MenuItem("Model"))
				{
					entity = scene->CreateEntity("Model");
					entity.AddComponent<ModelComponent>();
				}
				if (ImGui::MenuItem("Skinned Model"))
				{
					entity = scene->CreateEntity("Skinned Model");
					entity.AddComponent<SkinnedModelComponent>();
				}
				if (ImGui::MenuItem("Sprite"))
				{
					entity = scene->CreateEntity("Sprite");
					entity.AddComponent<SpriteComponent>();
				}
				if (ImGui::MenuItem("Particle Emitter"))
				{
					entity = scene->CreateEntity("Particle Emitter");
					entity.AddComponent<ParticleEmitterComponent>();
				}
				if (ImGui::MenuItem("Light"))
				{
					entity = scene->CreateEntity("Light");
					entity.AddComponent<LightComponent>();
				}
				if (ImGui::MenuItem("Quake Map"))
				{
					entity = scene->CreateEntity("Quake Map");
					entity.AddComponent<QuakeMapComponent>();
				}
				if (ImGui::MenuItem("NavMesh Volume"))
				{
					entity = scene->CreateEntity("NavMesh Volume");
					entity.AddComponent<NavMeshVolumeComponent>();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Physics"))
			{
				if (ImGui::MenuItem("Character Controller"))
				{
					entity = scene->CreateEntity("Character Controller");
					entity.AddComponent<CharacterControllerComponent>();
				}
				if (ImGui::MenuItem("Rigid Body"))
				{
					entity = scene->CreateEntity("Rigid Body");
					entity.AddComponent<RigidBodyComponent>();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Colliders"))
			{
				if (ImGui::MenuItem("Box Collider"))
				{
					entity = scene->CreateEntity("Box Collider");
					entity.AddComponent<BoxColliderComponent>();
				}
				if (ImGui::MenuItem("Sphere Collider"))
				{
					entity = scene->CreateEntity("Sphere Collider");
					entity.AddComponent<SphereColliderComponent>();
				}
				if (ImGui::MenuItem("Capsule Collider"))
				{
					entity = scene->CreateEntity("Capsule Collider");
					entity.AddComponent<CapsuleColliderComponent>();
				}
				if (ImGui::MenuItem("Cylinder Collider"))
				{
					entity = scene->CreateEntity("Cylinder Collider");
					entity.AddComponent<CylinderColliderComponent>();
				}
				if (ImGui::MenuItem("Mesh Collider"))
				{
					entity = scene->CreateEntity("Mesh Collider");
					entity.AddComponent<MeshColliderComponent>();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Audio"))
			{
				if (ImGui::MenuItem("Audio Emitter"))
				{
					entity = scene->CreateEntity("Audio Emitter");
					entity.AddComponent<AudioEmitterComponent>();
				}
				ImGui::EndMenu();
			}

			if (entity.IsValid())
			{
				if (Selection.Type == EditorSelectionType::Entity && Selection.Entity.IsValid())
				{
					Selection.Entity.AddChild(entity);
				}
				else
				{
					auto& camera = Engine::GetCurrentScene()->m_EditorCamera;
					Vector3 newEntityPos = camera->Translation + camera->Direction;
					entity.GetComponent<TransformComponent>().SetLocalPosition(newEntityPos);
				}

				Selection = EditorSelection(entity);
			}

			ImGui::EndPopup();
		}

		// Draw a tree of entities.
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(26.f / 255.0f, 26.f / 255.0f, 26.f / 255.0f, 1));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4, 4 });
		if (ImGui::BeginChild("Scene tree", ImGui::GetContentRegionAvail(), false))
		{
			if (ImGui::BeginTable("entity_table", 4, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp))
			{
				ImGui::TableSetupColumn("   Label", ImGuiTableColumnFlags_IndentEnable | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_IndentDisable | ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("Script", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_IndentDisable | ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("Visibility   ", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_IndentDisable | ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableHeadersRow();

				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));
				std::vector<Nuake::Entity> entities = scene->GetAllEntities();
				for (auto& e : entities)
				{
					ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanFullWidth;
					std::string name = e.GetComponent<NameComponent>().Name;
					// If selected add selected flag.
					if (Selection.Type == EditorSelectionType::Entity && Selection.Entity == e)
						base_flags |= ImGuiTreeNodeFlags_Selected;

					// Draw all entity without parents.
					if (!e.GetComponent<ParentComponent>().HasParent)
					{
						// Recursively draw childrens.
						DrawEntityTree(e);
					}

					// Pop font.
					//ImGui::PopFont();

					// Right click menu
					//if (ImGui::BeginPopupContextItem())
					//    ImGui::EndPopup();
				}
				ImGui::PopStyleVar();
			}
			ImGui::EndTable();

		}
		ImGui::EndChild();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
	}
	ImGui::End();

	SelectionPanel.Draw(Selection, prefab->Path);

	DrawViewportWindow();
}

void PrefabEditorWindow::DrawViewportWindow()
{
	
	//viewportFramebuffer->Bind();
	////RenderCommand::SetClearColor(Color(1, 0, 0, 1));
	//viewportFramebuffer->Clear();
	//viewportFramebuffer->Unbind();

	//RenderScene();

	if (ImGui::Begin(std::string("Viewport##" + prefab->Path).c_str()))
	{
		ImVec2 regionAvail = ImGui::GetContentRegionAvail();
		Vector2 viewportPanelSize = glm::vec2(regionAvail.x, regionAvail.y);

		if (viewportFramebuffer->GetSize() != viewportPanelSize)
			viewportFramebuffer->QueueResize(viewportPanelSize);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));\
			viewportFramebuffer->QueueResize(viewportPanelSize);
		ImGui::Image((void*)viewportFramebuffer->GetTexture(GL_COLOR_ATTACHMENT0)->GetID(), regionAvail, ImVec2(0, 1), ImVec2(1, 0));
		ImGui::PopStyleVar();

		const Vector2& mousePos = Input::GetMousePosition();

		const ImVec2& windowPos = ImGui::GetWindowPos();
		const auto windowPosNuake = Vector2(windowPos.x, windowPos.y);
		const ImVec2& windowSize = ImGui::GetWindowSize();
		const bool isInsideWidth = mousePos.x > windowPos.x && mousePos.x < windowPos.x + windowSize.x;
		const bool isInsideHeight = mousePos.y > windowPos.y && mousePos.y < windowPos.y + windowSize.y;
		isHoveringViewport = isInsideWidth && isInsideHeight;

		auto& editorCam = virtualScene->m_EditorCamera;
		bool isControllingCamera = editorCam->Update(Engine::GetTimestep(), isHoveringViewport);

		if (ImGui::IsWindowHovered() && isHoveringViewport && !isViewportFocused && ImGui::GetIO().WantCaptureMouse)
		{
			ImGui::FocusWindow(ImGui::GetCurrentWindow());
		}

		isViewportFocused = ImGui::IsWindowFocused();


		if (ImGui::GetIO().WantCaptureMouse && isHoveringViewport && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && isViewportFocused)
		{
			auto& gbuffer = virtualScene->m_SceneRenderer->GetGBuffer();
			auto pixelPos = (Input::GetMousePosition() - windowPosNuake);
			pixelPos.y = gbuffer.GetSize().y - pixelPos.y; // imgui coords are inverted on the Y axi

			gbuffer.Bind();
			if (const int result = gbuffer.ReadPixel(3, pixelPos); result > 0)
			{
				auto ent = Nuake::Entity{ (entt::entity)(result - 1), virtualScene.get() };
				if (ent.IsValid())
				{
					Selection = EditorSelection(ent);
					virtualScene->m_SceneRenderer->mOutlineEntityID = (uint32_t)ent.GetHandle();
				}
			}
			else
			{
				Selection = EditorSelection(); // None
			}

			gbuffer.Unbind();
		}
	}
	ImGui::End();
}

void PrefabEditorWindow::RenderScene()
{
	virtualScene->Update(Engine::GetTimestep());

	Ref<SceneRenderer> sceneRenderer = virtualScene->m_SceneRenderer;
	Ref<EditorCamera> editorCam = virtualScene->m_EditorCamera;
	editorCam->OnWindowResize(viewportFramebuffer->GetSize().x, viewportFramebuffer->GetSize().y);
	virtualScene->Draw(*viewportFramebuffer.get());

	sceneRenderer->BeginRenderScene(editorCam->GetPerspective(), editorCam->GetTransform(), editorCam->Translation);
	sceneRenderer->RenderScene(*virtualScene, *viewportFramebuffer.get(), true);

}

void PrefabEditorWindow::DrawEntityTree(Nuake::Entity e)
{
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ 0.0f, 0.0f });

	ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAllColumns;

	NameComponent& nameComponent = e.GetComponent<NameComponent>();
	std::string name = nameComponent.Name;

	ParentComponent& parent = e.GetComponent<ParentComponent>();

	if (Selection.Type == EditorSelectionType::Entity && Selection.Entity == e)
		base_flags |= ImGuiTreeNodeFlags_Selected;

	ImGui::TableNextColumn();

	// Write in normal font.
	// ImGui::PushFont(normalFont);

	// If has no childrens draw tree node leaf
	if (parent.Children.size() <= 0)
	{
		base_flags |= ImGuiTreeNodeFlags_Leaf;
	}

	if (nameComponent.IsPrefab && e.HasComponent<PrefabComponent>())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
	}
	else if (e.HasComponent<BSPBrushComponent>())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 120));
	}

	//if (!m_IsRenaming && m_ShouldUnfoldEntityTree && Selection.Type == EditorSelectionType::Entity && e.GetScene()->EntityIsParent(Selection.Entity, e))
	//{
	//	ImGui::SetNextItemOpen(true);
	//}

	auto cursorPos = ImGui::GetCursorPos();
	ImGui::SetNextItemAllowOverlap();
	bool open = ImGui::TreeNodeEx(name.c_str(), base_flags);

	if (isRenaming)
	{
		if (Selection.Type == EditorSelectionType::Entity && Selection.Entity == e)
		{
			ImGui::SetCursorPosY(cursorPos.y);
			ImGui::Indent();
			ImGui::InputText("##renamingEntity", &name);
			ImGui::Unindent();
			if (Input::IsKeyDown(Key::ENTER))
			{
				nameComponent.Name = name;
				isRenaming = false;
			}
		}
	}

	bool isDragging = false;
	if (nameComponent.IsPrefab && e.HasComponent<PrefabComponent>() || e.HasComponent<BSPBrushComponent>())
	{
		ImGui::PopStyleColor();
	}
	else if (!isRenaming && ImGui::BeginDragDropSource())
	{
		ImGui::SetDragDropPayload("ENTITYPrefab", (void*)&e, sizeof(Nuake::Entity));
		ImGui::Text(name.c_str());
		ImGui::EndDragDropSource();
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITYPrefab"))
		{
			Nuake::Entity payload_entity = *(const Nuake::Entity*)payload->Data;

			// Check if entity is already parent.
			ParentComponent& parentPayload = payload_entity.GetComponent<ParentComponent>();
			if (!payload_entity.EntityContainsItself(payload_entity, e) && parentPayload.Parent != e && std::count(parent.Children.begin(), parent.Children.end(), payload_entity) == 0)
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
		else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_NetScript"))
		{
			char* file = (char*)payload->Data;

			std::string fullPath = std::string(file, 512);
			std::string path = Nuake::FileSystem::AbsoluteToRelative(std::move(fullPath));

			if (e.HasComponent<NetScriptComponent>())
			{
				e.GetComponent<NetScriptComponent>().ScriptPath = path;
			}
			else
			{
				e.AddComponent<NetScriptComponent>().ScriptPath = path;
			}
		}
		ImGui::EndDragDropTarget();
	}


	if (!isDragging && ImGui::IsItemHovered() && ImGui::IsMouseReleased(0))
	{
		// We selected another another that we werent renaming
		if (Selection.Entity != e)
		{
			isRenaming = false;
		}

		Selection = EditorSelection(e);
	}

	if (!isDragging && (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) || Input::IsKeyPressed(Key::F2))
	{
		isRenaming = true;
	}

	if (!isRenaming && Selection.Type == EditorSelectionType::Entity && Input::IsKeyPressed(Key::DELETE_KEY))
	{
		//QueueDeletion = Selection.Entity;
	}

	if (ImGui::BeginPopupContextItem())
	{
		Selection = EditorSelection(e);

		Nuake::Entity entity = Selection.Entity;
		if (entity.HasComponent<CameraComponent>())
		{
			// Moves the editor camera to the camera position and direction.
			if (ImGui::Selectable("Focus camera"))
			{
				Ref<EditorCamera> editorCam = Engine::GetCurrentScene()->m_EditorCamera;
				Vector3 camDirection = entity.GetComponent<CameraComponent>().CameraInstance->GetDirection();
				camDirection.z *= -1.0f;
				editorCam->SetTransform(glm::inverse(entity.GetComponent<TransformComponent>().GetGlobalTransform()));
			}
			ImGui::Separator();
		}

		if (ImGui::Selectable("Remove"))
		{
			//QueueDeletion = e;
		}

		if (entity.GetComponent<ParentComponent>().HasParent && ImGui::Selectable("Move to root"))
		{
			auto& parentComp = Selection.Entity.GetComponent<ParentComponent>();
			if (parentComp.HasParent)
			{
				auto& parentParentComp = parentComp.Parent.GetComponent<ParentComponent>();
				parentParentComp.RemoveChildren(entity);
				parentComp.HasParent = false;
			}
		}

		if (ImGui::Selectable("Save entity as a new prefab"))
		{
			Ref<Prefab> newPrefab = Prefab::CreatePrefabFromEntity(Selection.Entity);
			std::string savePath = Nuake::FileDialog::SaveFile("*.prefab");
			if (!String::EndsWith(savePath, ".prefab"))
			{
				savePath += ".prefab";
			}

			if (!savePath.empty())
			{
				newPrefab->SaveAs(savePath);
				Selection.Entity.AddComponent<PrefabComponent>().PrefabInstance = newPrefab;
			}
		}
		ImGui::EndPopup();
	}

	ImGui::TableNextColumn();

	ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1.0), "");

	ImGui::TableNextColumn();
	{
		bool hasScript = e.HasComponent<NetScriptComponent>();
		if (hasScript)
		{
			std::string scrollIcon = std::string(ICON_FA_SCROLL) + "##" + name;
			ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
			if (ImGui::Button(scrollIcon.c_str(), { 40, 0 }))
			{
				auto& scriptComponent = e.GetComponent<NetScriptComponent>();
				if (!scriptComponent.ScriptPath.empty() && FileSystem::FileExists(scriptComponent.ScriptPath))
				{
					OS::OpenIn(FileSystem::RelativeToAbsolute(scriptComponent.ScriptPath));
				}
			}
			ImGui::PopStyleColor();
		}
	}

	ImGui::TableNextColumn();
	{
		bool& isVisible = e.GetComponent<VisibilityComponent>().Visible;
		std::string visibilityIcon = isVisible ? ICON_FA_EYE : ICON_FA_EYE_SLASH;
		ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
		if (ImGui::Button(visibilityIcon.c_str(), { 40, 0 }))
		{
			isVisible = !isVisible;
		}
		ImGui::PopStyleColor();
	}

	if (open)
	{
		// Caching list to prevent deletion while iterating.
		std::vector<Nuake::Entity> childrens = parent.Children;
		for (auto& c : childrens)
			DrawEntityTree(c);

		ImGui::TreePop();
	}

	ImGui::PopStyleVar();
	//ImGui::PopFont();
}

#include "Nuake/Core/Core.h"
#include "SceneHierarchyWidget.h"

#include "Nuake/UI/ImUI.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include "Nuake/Scene/Entities/Entity.h"
#include "Nuake/Scene/Components.h"


#include <Nuake/Core/Input.h>
#include <Nuake/FileSystem/FileDialog.h>
#include "Engine.h"
#include "Nuake/Resource/Project.h"

using namespace Nuake;

SceneHierarchyWidget::SceneHierarchyWidget(EditorContext& inCtx) : 
	IEditorWidget(inCtx),
	isRenaming(false),
	searchQuery("")
{

}

void SceneHierarchyWidget::Update(float ts)
{
	
}

void SceneHierarchyWidget::Draw()
{
	if (BeginWidgetWindow("Scene Hierarchy"))
	{
		DrawSearchBar();

		DrawCreateEntityButton();

		DrawEntityTree();
	}
	ImGui::End();
}

void SceneHierarchyWidget::DrawSearchBar()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 8, 8 });
	ImGui::InputTextWithHint("##search", "Search entity", &searchQuery, 0, 0, 0);
	ImGui::PopStyleVar();
	ImGui::SameLine();
}

void SceneHierarchyWidget::DrawCreateEntityButton()
{
	Ref<Scene> scene = editorContext.GetScene();
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
			EditorSelection selection = editorContext.GetSelection();
			if (selection.Type == EditorSelectionType::Entity && selection.Entity.IsValid())
			{
				selection.Entity.AddChild(entity);
			}
			else
			{
				auto& camera = editorContext.GetScene()->m_EditorCamera;
				Vector3 newEntityPos = camera->Translation + camera->Direction;
				entity.GetComponent<TransformComponent>().SetLocalPosition(newEntityPos);
			}

			editorContext.SetSelection(EditorSelection(entity));
		}

		ImGui::EndPopup();
	}
}

void SceneHierarchyWidget::DrawEntityTree()
{
	Ref<Scene> scene = editorContext.GetScene();

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

			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));

			std::vector<Nuake::Entity> entitiesToDisplay;
			if (searchQuery.empty())
			{
				entitiesToDisplay = scene->GetAllEntities();
			}
			else
			{
				auto view = scene->m_Registry.view<NameComponent>();
				for (auto& e : view)
				{
					auto& nameComponent = view.get<NameComponent>(e);
					if (String::RemoveWhiteSpace(String::ToUpper(nameComponent.Name)).find(String::RemoveWhiteSpace(String::ToUpper(searchQuery))) != std::string::npos)
					{
						entitiesToDisplay.push_back({ e, scene.get() });
					}
				}
			}

			// Display valid entities
			for (Nuake::Entity e : entitiesToDisplay)
			{
				// Draw all entity without parents.
				bool displayAllHierarchy = searchQuery.empty();
				if ((displayAllHierarchy && !e.GetComponent<ParentComponent>().HasParent) || !displayAllHierarchy)
				{
					// TODO(antopilo): Add font regular font
					// ImGui::PushFont(normalFont);

					// Recursively draw childrens if not searching
					DrawEntity(e, displayAllHierarchy);

					// ImGui::PopFont();
				}
			}
			ImGui::PopStyleVar();
		}
		ImGui::EndTable();
	}

	ImGui::EndChild();
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();

	if(ImGui::BeginDragDropTarget()) // Drag n drop new prefab file into scene tree
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Prefab"))
		{
			char* file = (char*)payload->Data;
			std::string fullPath = std::string(file, 256);
			std::string relPath = FileSystem::AbsoluteToRelative(fullPath);
			auto newPrefabInstance = Prefab::New(relPath);
			newPrefabInstance->Root.GetComponent<PrefabComponent>().SetPrefab(newPrefabInstance);
			newPrefabInstance->Root.GetComponent<NameComponent>().IsPrefab = true;
		}
		ImGui::EndDragDropTarget();
	}

	if (deletionQueue.GetHandle() != -1)
	{
		scene->DestroyEntity(deletionQueue);

		// Unselect
		editorContext.SetSelection(EditorSelection());

		deletionQueue = Nuake::Entity{ (entt::entity)-1, scene.get() };
	}
}

void SceneHierarchyWidget::DrawEntity(Nuake::Entity entity, bool drawChildrens)
{
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ 0.0f, 0.0f });
	ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAllColumns;
	
	NameComponent& nameComponent = entity.GetComponent<NameComponent>();
	std::string name = nameComponent.Name;

	ParentComponent& parent = entity.GetComponent<ParentComponent>();

	ImGui::TableNextColumn();

	// Draw first column Name
	
	// Highlight if the entity is currently selected
	auto selection = editorContext.GetSelection();
	if (selection.Type == EditorSelectionType::Entity && selection.Entity == entity)
	{
		base_flags |= ImGuiTreeNodeFlags_Selected;
	}

	// Display arrow if it has children
	bool isPrefab = entity.HasComponent<PrefabComponent>();
	if (parent.Children.size() == 0 || isPrefab || !drawChildrens)
	{
		base_flags |= ImGuiTreeNodeFlags_Leaf;
	}

	// Hightlight if the entity is a prefab or BSP
	if (nameComponent.IsPrefab && entity.HasComponent<PrefabComponent>())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
	}
	else if (entity.HasComponent<BSPBrushComponent>())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 120));
	}

	// TODO(antopilo) unfolding when mouse picking


	ImVec2 cursorPos = ImGui::GetCursorPos();
	ImVec2 cursorPosition = ImGui::GetCursorScreenPos();
	const auto& cleanName = String::RemoveWhiteSpace(String::ToUpper(name));
	const size_t searchIt = cleanName.find(String::RemoveWhiteSpace(String::ToUpper(searchQuery)));

	ImGui::SetNextItemAllowOverlap();
	bool open = ImGui::TreeNodeEx(name.c_str(), base_flags);

	if (!searchQuery.empty() && searchIt != std::string::npos)
	{
		int firstLetterFoundIndex = static_cast<int>(searchIt);

		const auto foundStr = name.substr(0, firstLetterFoundIndex + searchQuery.size());
		auto highlightBeginPos = ImGui::CalcTextSize(foundStr.c_str());
		auto highlightEndPos = ImGui::CalcTextSize(searchQuery.c_str());

		auto fg = ImGui::GetForegroundDrawList();
		auto color = Engine::GetProject()->Settings.PrimaryColor;
		auto rgbColor = IM_COL32(color.r * 255.0f, color.g * 255.0f, color.b * 255.0f, std::min(color.a, 0.2f) * 255.0f);

		fg->AddRectFilled(ImVec2(cursorPosition.x + 20.0f, cursorPosition.y + 4.0f), ImVec2(cursorPosition.x + highlightEndPos.x + 26.0f, cursorPosition.y + highlightEndPos.y + 6.0f), rgbColor, 4.0f);
	}

	// Renaming behavior
	if (isRenaming)
	{
		if (selection.Type == EditorSelectionType::Entity && selection.Entity == entity)
		{
			ImGui::SetCursorPosY(cursorPos.y);
			ImGui::Indent();
			ImGui::InputText("##renamingEntity", &name);
			ImGui::Unindent();
			if (Nuake::Input::IsKeyDown(Key::ENTER))
			{
				nameComponent.Name = name;
				isRenaming = false;
			}
		}
	}

	// Drag and drop behavior
	bool isDragging = false;
	if (nameComponent.IsPrefab && entity.HasComponent<PrefabComponent>() || entity.HasComponent<BSPBrushComponent>())
	{
		ImGui::PopStyleColor();
	}
	else if (!isRenaming && ImGui::BeginDragDropSource())
	{
		ImGui::SetDragDropPayload("ENTITYPrefab", (void*)&entity, sizeof(Nuake::Entity));
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
			if (!payload_entity.EntityContainsItself(payload_entity, entity) && parentPayload.Parent != entity && std::count(parent.Children.begin(), parent.Children.end(), payload_entity) == 0)
			{
				if (parentPayload.HasParent)
				{
					// Erase remove idiom.
					ParentComponent& childOfParent = parentPayload.Parent.GetComponent<ParentComponent>();
					childOfParent.Children.erase(std::remove(childOfParent.Children.begin(), childOfParent.Children.end(), payload_entity), childOfParent.Children.end());
				}

				parentPayload.Parent = entity;
				parentPayload.HasParent = true;
				parent.Children.push_back(payload_entity);
			}
		}
		else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_NetScript"))
		{
			char* file = (char*)payload->Data;

			std::string fullPath = std::string(file, 512);
			std::string path = Nuake::FileSystem::AbsoluteToRelative(std::move(fullPath));

			if (entity.HasComponent<NetScriptComponent>())
			{
				entity.GetComponent<NetScriptComponent>().ScriptPath = path;
			}
			else
			{
				entity.AddComponent<NetScriptComponent>().ScriptPath = path;
			}
		}
		ImGui::EndDragDropTarget();
	}

	if (!isDragging && ImGui::IsItemHovered() && ImGui::IsMouseReleased(0))
	{
		// We selected another another that we werent renaming
		if (selection.Entity != entity)
		{
			isRenaming = false;
		}

		editorContext.SetSelection(EditorSelection(entity));
	}

	if (!isDragging && (ImGui::IsItemHovered() && ImGui::IsMouseTripleClicked(0)) || Input::IsKeyPressed(Key::F2))
	{
		isRenaming = true;
	}

	if (!isDragging && (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) || Input::IsKeyPressed(Key::F2))
	{
		Ref<EditorCamera> editorCam = editorContext.GetScene()->m_EditorCamera;

		// Get target object's world position
		glm::vec3 targetPos = glm::vec3(entity.GetComponent<TransformComponent>().GetGlobalTransform()[3]);

		// Choose a direction and distance to place the camera away from the object
		glm::vec3 offset = glm::vec3(2.0f, 2.0f, 2.0f); // above and behind

		// Place camera at offset from the target
		glm::vec3 camPos = targetPos + offset;

		editorCam->Translation = camPos;

		Vector3 direction = glm::normalize(targetPos - Vector3(camPos));
		float yaw = glm::degrees(atan2(direction.z, direction.x));
		float pitch = glm::degrees(asin(direction.y));

		editorCam->TargetYaw = yaw;
		editorCam->TargetPitch = pitch;
		// Set the camera's transform as the inverse of the view matrix
		//editorCam->SetTransform(glm::inverse(view));
	}

	if (!isRenaming && selection.Type == EditorSelectionType::Entity && Input::IsKeyPressed(Key::DELETE_KEY))
	{
		this->deletionQueue = selection.Entity;
	}

	if (ImGui::BeginPopupContextItem())
	{
		editorContext.SetSelection(EditorSelection(entity));

		Nuake::Entity entity = selection.Entity;
		if (entity.HasComponent<CameraComponent>())
		{
			// Moves the editor camera to the camera position and direction.
			if (ImGui::Selectable("Focus camera"))
			{
				Ref<EditorCamera> editorCam = editorContext.GetScene()->m_EditorCamera;
				Vector3 camDirection = entity.GetComponent<CameraComponent>().CameraInstance->GetDirection();
				camDirection.z *= -1.0f;
				editorCam->SetTransform(glm::inverse(entity.GetComponent<TransformComponent>().GetGlobalTransform()));
			}
			ImGui::Separator();
		}

		if (ImGui::Selectable("Remove"))
		{
			deletionQueue = entity;
		}

		if (entity.GetComponent<ParentComponent>().HasParent && ImGui::Selectable("Move to root"))
		{
			auto& parentComp = selection.Entity.GetComponent<ParentComponent>();
			if (parentComp.HasParent)
			{
				auto& parentParentComp = parentComp.Parent.GetComponent<ParentComponent>();
				parentParentComp.RemoveChildren(entity);
				parentComp.HasParent = false;
			}
		}

		if (ImGui::Selectable("Save entity as a new prefab"))
		{
			Ref<Prefab> newPrefab = Prefab::CreatePrefabFromEntity(selection.Entity);
			std::string savePath = Nuake::FileDialog::SaveFile("*.prefab");
			if (!String::EndsWith(savePath, ".prefab"))
			{
				savePath += ".prefab";
			}

			if (!savePath.empty())
			{
				newPrefab->SaveAs(savePath);
				selection.Entity.AddComponent<PrefabComponent>().PrefabInstance = newPrefab;
			}
		}
		ImGui::EndPopup();
	}
	ImGui::TableNextColumn();

	ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1.0), "");

	ImGui::TableNextColumn();

	{
		bool hasScript = entity.HasComponent<NetScriptComponent>();
		if (hasScript)
		{
			std::string scrollIcon = std::string(ICON_FA_SCROLL) + "##" + name;
			ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
			if (ImGui::Button(scrollIcon.c_str(), { 40, 0 }))
			{
				auto& scriptComponent = entity.GetComponent<NetScriptComponent>();
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
		bool& isVisible = entity.GetComponent<VisibilityComponent>().Visible;
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
		if (drawChildrens && !isPrefab)
		{
			// Caching list to prevent deletion while iterating.
			std::vector<Nuake::Entity> childrens = parent.Children;
			for (auto& e : childrens)
			{
				DrawEntity(e);
			}
		}

		ImGui::TreePop();
	}

	ImGui::PopStyleVar();
}
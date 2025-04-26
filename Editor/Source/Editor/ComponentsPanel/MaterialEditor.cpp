#include "MaterialEditor.h"
#include "../Misc/InterfaceFonts.h"
#include <Nuake/Resource/FontAwesome5.h>
#include <Nuake/Resource/ResourceManager.h>
#include <Nuake/FileSystem/FileDialog.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <Nuake/Scene/Components/CameraComponent.h>
#include "Nuake/Rendering/Vulkan/SceneViewport.h"
#include "Nuake/Scene/Entities/Entity.h"
#include <Nuake/Scene/Components/ModelComponent.h>
#include <Engine.h>
#include "Nuake/Rendering/Vulkan/DebugCmd.h"
#include "Nuake/Resource/Project.h"
#include <Nuake/UI/ImUI.h>

Ref<Nuake::Model> sphereModel;
Ref<Nuake::Model> quadModel;
Ref<Nuake::Model> cubeModel;

enum class Shapes
{
	Sphere, Cube, Quad
};

Shapes currentShape = Shapes::Sphere;

void MaterialEditor::Enable()
{
	SceneViewport->SetActive(true);
}

void MaterialEditor::Disable()
{
	SceneViewport->SetActive(false);
}

MaterialEditor::MaterialEditor()
{
	using namespace Nuake;

	PreviewScene = CreateRef<Nuake::Scene>();
	PreviewScene->GetEnvironment()->mVolumetric->SetFogAmount(1.0f);

	auto camera = PreviewScene->CreateEntity("Camera");
	camera.GetComponent<Nuake::TransformComponent>().Translation = Nuake::Vector3(-2, 0, -2);
	auto& camComponent = camera.AddComponent<Nuake::CameraComponent>();
	camComponent.CameraInstance->Fov = 44.0f;
	auto light = PreviewScene->CreateEntity("Light");
	auto& lightC = light.AddComponent<LightComponent>();
	auto& lightT = light.AddComponent<TransformComponent>();
	lightC.CastShadows = false;
	lightC.Type = LightType::Directional;

	glm::vec3 forward = glm::normalize(Vector3(.33, -.33, -.33));
	glm::vec3 up = glm::vec3(0, 1, 0);

	// Prevent up being colinear with forward
	if (glm::abs(glm::dot(forward, up)) > 0.999f)
		up = glm::vec3(1, 0, 0);

	glm::vec3 right = glm::normalize(glm::cross(up, forward));
	up = glm::cross(forward, right);

	glm::mat3 rotationMatrix(right, up, forward);

	//auto& lightT = light.GetComponent<TransformComponent>();
	lightT.GlobalRotation = glm::quat_cast(rotationMatrix);

	auto sphere = PreviewScene->CreateEntity("Sphere");
	auto& modelComponent = sphere.AddComponent<Nuake::ModelComponent>();
	auto& sphereT = sphere.GetComponent<TransformComponent>();
	sphereT.Translation = { 1, 0, 0 };
	const std::vector<Nuake::Vertex> quadVertices
	{
		{ Vector3(-0.5f,  0.5f, 0.0f), 0.0f, Vector3(0, 0, 1), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(0.5f,  0.5f, 0.0f), 1.0f, Vector3(0, 0, 1), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(-0.5f, -0.5f, 0.0f), 0.0f, Vector3(0, 0, 1), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(0.5f, -0.5f, 0.0f), 1.0f, Vector3(0, 0, 1), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(-0.5f, -0.5f, 0.0f), 0.0f, Vector3(0, 0, 1), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(0.5f,  0.5f, 0.0f), 1.0f, Vector3(0, 0, 1), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) }
	};

	const std::vector<uint32_t> quadIndices
	{
		5, 4, 3, 2, 1, 0
	};

	auto meshQuad = CreateRef<Mesh>();
	meshQuad->AddSurface(quadVertices, quadIndices);

	quadModel = CreateRef<Model>();
	quadModel->AddMesh(meshQuad);
	ResourceManager::RegisterResource(quadModel);
	
	// Sphere
	const int sectorCount = 32;
	const int stackCount = 16;
	const float radius = 0.5f;

	std::vector<Nuake::Vertex> sphereVertices;
	std::vector<uint32_t> sphereIndices;

	for (int i = 0; i <= stackCount; ++i)
	{
		float stackAngle = glm::pi<float>() / 2 - i * glm::pi<float>() / stackCount;
		float xy = radius * cosf(stackAngle);
		float z = radius * sinf(stackAngle);

		for (int j = 0; j <= sectorCount; ++j)
		{
			float sectorAngle = j * 2.0f * glm::pi<float>() / sectorCount;

			float x = xy * cosf(sectorAngle);
			float y = xy * sinf(sectorAngle);
			Vector3 position = Vector3(x, y, z);
			Vector3 normal = glm::normalize(position);

			// Collapse seam at the poles
			float u = (i == 0 || i == stackCount) ? 0.5f : (float)j / sectorCount;
			float v = 1.0f - (float)i / stackCount;

			Vector4 tangent = Vector4(1, 0, 0, 0);
			Vector4 bitangent = Vector4(0, 1, 0, 0);

			sphereVertices.push_back({ position, u, normal, v, tangent, bitangent });
		}
	}

	for (int i = 0; i < stackCount; ++i)
	{
		int k1 = i * (sectorCount + 1);
		int k2 = k1 + sectorCount + 1;

		for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
		{
			if (i != 0)
			{
				sphereIndices.push_back(k1);
				sphereIndices.push_back(k2);
				sphereIndices.push_back(k1 + 1);
			}

			if (i != (stackCount - 1))
			{
				sphereIndices.push_back(k1 + 1);
				sphereIndices.push_back(k2);
				sphereIndices.push_back(k2 + 1);
			}
		}
	}

	auto mesh = CreateRef<Mesh>();
	mesh->AddSurface(sphereVertices, sphereIndices);

	sphereModel = CreateRef<Model>();
	sphereModel->AddMesh(mesh);
	ResourceManager::RegisterResource(sphereModel);
	modelComponent.ModelResource = sphereModel->ID;

	const float boxSize = 0.33f;
	const std::vector<Nuake::Vertex> boxVertices =
	{
		// Front face
		{ Vector3(-boxSize, -boxSize,  boxSize), 0.0f, Vector3(0, 0, 1), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(boxSize, -boxSize,  boxSize), 1.0f, Vector3(0, 0, 1), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(boxSize,  boxSize,  boxSize), 1.0f, Vector3(0, 0, 1), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(-boxSize,  boxSize,  boxSize), 0.0f, Vector3(0, 0, 1), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0) },

		// Back face
		{ Vector3(-boxSize, -boxSize, -boxSize), 1.0f, Vector3(0, 0, -1), 0.0f, Vector4(-1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(-boxSize,  boxSize, -boxSize), 1.0f, Vector3(0, 0, -1), 1.0f, Vector4(-1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(boxSize,  boxSize, -boxSize), 0.0f, Vector3(0, 0, -1), 1.0f, Vector4(-1, 0, 0, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(boxSize, -boxSize, -boxSize), 0.0f, Vector3(0, 0, -1), 0.0f, Vector4(-1, 0, 0, 0), Vector4(0, 1, 0, 0) },

		// Left face
		{ Vector3(-boxSize, -boxSize, -boxSize), 0.0f, Vector3(-1, 0, 0), 0.0f, Vector4(0, 0, 1, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(-boxSize, -boxSize,  boxSize), 1.0f, Vector3(-1, 0, 0), 0.0f, Vector4(0, 0, 1, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(-boxSize,  boxSize,  boxSize), 1.0f, Vector3(-1, 0, 0), 1.0f, Vector4(0, 0, 1, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(-boxSize,  boxSize, -boxSize), 0.0f, Vector3(-1, 0, 0), 1.0f, Vector4(0, 0, 1, 0), Vector4(0, 1, 0, 0) },

		// Right face
		{ Vector3(boxSize, -boxSize, -boxSize), 1.0f, Vector3(1, 0, 0), 0.0f, Vector4(0, 0, -1, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(boxSize,  boxSize, -boxSize), 1.0f, Vector3(1, 0, 0), 1.0f, Vector4(0, 0, -1, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(boxSize,  boxSize,  boxSize), 0.0f, Vector3(1, 0, 0), 1.0f, Vector4(0, 0, -1, 0), Vector4(0, 1, 0, 0) },
		{ Vector3(boxSize, -boxSize,  boxSize), 0.0f, Vector3(1, 0, 0), 0.0f, Vector4(0, 0, -1, 0), Vector4(0, 1, 0, 0) },

		// Top face
		{ Vector3(-boxSize,  boxSize, -boxSize), 0.0f, Vector3(0, 1, 0), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 0, -1, 0) },
		{ Vector3(-boxSize,  boxSize,  boxSize), 0.0f, Vector3(0, 1, 0), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 0, -1, 0) },
		{ Vector3(boxSize,  boxSize,  boxSize), 1.0f, Vector3(0, 1, 0), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 0, -1, 0) },
		{ Vector3(boxSize,  boxSize, -boxSize), 1.0f, Vector3(0, 1, 0), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 0, -1, 0) },

		// Bottom face
		{ Vector3(-boxSize, -boxSize, -boxSize), 1.0f, Vector3(0, -1, 0), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 0, 1, 0) },
		{ Vector3(boxSize, -boxSize, -boxSize), 0.0f, Vector3(0, -1, 0), 0.0f, Vector4(1, 0, 0, 0), Vector4(0, 0, 1, 0) },
		{ Vector3(boxSize, -boxSize,  boxSize), 0.0f, Vector3(0, -1, 0), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 0, 1, 0) },
		{ Vector3(-boxSize, -boxSize,  boxSize), 1.0f, Vector3(0, -1, 0), 1.0f, Vector4(1, 0, 0, 0), Vector4(0, 0, 1, 0) },
	};

	const std::vector<uint32_t> boxIndices =
	{
		// Front
		0, 1, 2, 2, 3, 0,
		// Back
		4, 5, 6, 6, 7, 4,
		// Left
		8, 9,10,10,11, 8,
		// Right
		12,13,14,14,15,12,
		// Top
		16,17,18,18,19,16,
		// Bottom
		20,21,22,22,23,20
	};

	auto meshBox = CreateRef<Mesh>();
	meshBox->AddSurface(boxVertices, boxIndices);

	cubeModel = CreateRef<Model>();
	cubeModel->AddMesh(meshBox);
	ResourceManager::RegisterResource(cubeModel);
	modelComponent.ModelResource = cubeModel->ID;

	auto& vkRenderer = Nuake::VkRenderer::Get();
	const UUID viewId = camComponent.CameraInstance->ID;

	SceneViewport = vkRenderer.CreateViewport(viewId, {200, 200});
	SceneViewport->GetOnDebugDraw().AddStatic([](DebugCmd& cmd) {
		Matrix4 transform = Matrix4(1.0f);
		//cmd.DrawQuad(transform);
	});
	SceneViewport->GetOnLineDraw().AddStatic([](DebugLineCmd& cmd) {
		Matrix4 transform = Matrix4(1.0f);

		auto cam = cmd.GetScene()->GetEntity("Camera");
		auto& camc = cam.GetComponent<CameraComponent>();
		auto& cami = camc.CameraInstance;
		auto proj = cami->GetPerspective();
		auto view = cami->GetTransform();
		//cmd.DrawSphere(proj * view * transform, { 1, 0, 0, 1 }, 2.0f, false);
	});

	//PreviewScene->GetEnvironment()->AmbientColor = { 0, 0, 0, 0 };
	vkRenderer.RegisterSceneViewport(PreviewScene, SceneViewport->GetID());
}

void MaterialEditor::Draw(Ref<Nuake::Material> material)
{
	using namespace Nuake;

	auto& t = PreviewScene->GetEntity("Camera").GetComponent<CameraComponent>();
	auto ent = PreviewScene->GetEntity("Sphere");

	RID shape;
	switch (currentShape)
	{
		case Shapes::Quad:	
		{
			shape = quadModel->ID;
			break;
		}
		case Shapes::Sphere:
		{
			shape = sphereModel->ID;
			break;
		}
		case Shapes::Cube:
		{
			shape = cubeModel->ID;
			break;
		}
	}

	shape.Get<Model>()->GetMeshes()[0]->SetMaterial(material);

	ent.GetComponent<ModelComponent>().ModelResource = shape;

	auto& cam = Engine::GetCurrentScene()->m_EditorCamera;
	
	std::string materialTitle = material->Path;
	{
		UIFont boldfont = UIFont(Fonts::SubTitle);
		ImGui::Text(material->Path.c_str());
	}
	ImGui::SameLine();
	{
		UIFont boldfont = UIFont(Fonts::Icons);
		if (ImGui::Button(ICON_FA_SAVE))
		{
			if (ResourceManager::IsResourceLoaded(material->ID))
			{
				ResourceManager::RegisterResource(material);
			}

			std::string fileData = material->Serialize().dump(4);

			FileSystem::BeginWriteFile(material->Path);
			FileSystem::WriteLine(fileData);
			FileSystem::EndWriteFile();
		}
	}

	ImVec2 previewSize = {ImGui::GetContentRegionAvail().x, 200 };
	Vector2 viewportPanelSize = glm::vec2(previewSize.x, previewSize.y);
	static float yaw = 0.0f;        // Horizontal angle in radians
	static float pitch = 0.0f;      // Vertical angle in radians
	static float actualYaw = 0.0f;
	static float actualPitch = 0.0f;
	static Vector3 actualPos = { 0, 0, 0 };

	static float distance = 1.5f;   // Distance from origin
	const float yawSpeed = 0.01f;
	const float pitchSpeed = 0.01f;
	static Vector3 camPosition = Vector3(1.5, 0, 0);
	t.CameraInstance->OnWindowResize(previewSize.x, previewSize.y);
	SceneViewport->QueueResize({ previewSize.x, previewSize.y });
	ImGui::BeginChild("MaterialPreview", previewSize);
	{
		auto cursorPos = ImGui::GetCursorPos();

		if (ImGui::BeginPopup("shapes_popup"))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));
			ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100);

			bool isActive = currentShape == Shapes::Sphere;
			if (isActive)
			{
				Color color = Engine::GetProject()->Settings.PrimaryColor;
				ImGui::PushStyleColor(ImGuiCol_Button, { color.r, color.g, color.b, 1.0f });
			}

			if (ImGui::Button((std::string(ICON_FA_CIRCLE) + " Sphere").c_str()))
			{
				currentShape = Shapes::Sphere;
			}

			Nuake::UI::Tooltip("Sphere");
			if (isActive)
			{
				ImGui::PopStyleColor();
			}

			isActive = currentShape == Shapes::Cube;
			if (isActive)
			{
				Color color = Engine::GetProject()->Settings.PrimaryColor;
				ImGui::PushStyleColor(ImGuiCol_Button, { color.r, color.g, color.b, 1.0f });
			}

			if (ImGui::Button((std::string(ICON_FA_CUBE) + " Cube").c_str()))
			{
				currentShape = Shapes::Cube;
			}

			UI::Tooltip("Cube");
			if (isActive)
			{
				ImGui::PopStyleColor();
			}
			
			isActive = currentShape == Shapes::Quad;
			if (isActive)
			{
				Color color = Engine::GetProject()->Settings.PrimaryColor;
				ImGui::PushStyleColor(ImGuiCol_Button, { color.r, color.g, color.b, 1.0f });
			}

			if (ImGui::Button((std::string(ICON_FA_SQUARE) + " Plane").c_str()))
			{
				currentShape = Shapes::Quad;
			}

			UI::Tooltip("Plane");
			if (isActive)
			{
				ImGui::PopStyleColor();
			}

			ImGui::PopStyleColor();
			ImGui::PopStyleVar(2);

			ImGui::EndPopup();
		}

		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
			yaw -= delta.x * yawSpeed;
			pitch += delta.y * pitchSpeed;

			// Clamp pitch to prevent flipping
			pitch = std::clamp(pitch, -glm::half_pi<float>() + 0.01f, glm::half_pi<float>() - 0.01f);

			ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left); // Prevents accumulating deltas
		}

		VkDescriptorSet textureDesc = SceneViewport->GetRenderTarget()->GetImGuiDescriptorSet();
		ImGui::Image(textureDesc, ImGui::GetContentRegionAvail(), {0, 1}, {1, 0});
		ImGui::SetCursorPos(cursorPos);

		if (ImGui::Button(ICON_FA_SHAPES, ImVec2(30, 28)))
		{
			ImGui::OpenPopup("shapes_popup");
		}

		UI::Tooltip("Preview Shape");
	}

	ImGui::EndChild();

	actualYaw = glm::mix(actualYaw, yaw, 20.0f * Engine::GetTimestep());
	actualPitch = glm::mix(actualPitch, pitch, 20.0f * Engine::GetTimestep());

	camPosition.x = distance * cosf(actualPitch) * sinf(actualYaw);
	camPosition.y = distance * sinf(actualPitch);
	camPosition.z = distance * cosf(actualPitch) * cosf(actualYaw);

	actualPos = glm::mix(actualPos, camPosition, 20.0f * Engine::GetTimestep());

	Vector3 target = Vector3(0, 0, 0);
	Vector3 direction = glm::normalize(target - camPosition);

	t.CameraInstance->Translation = { camPosition };
	t.CameraInstance->SetDirection(direction);
	bool flagsHeaderOpened;
	{
		UIFont boldfont = UIFont(Fonts::Bold);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 8.f));
		flagsHeaderOpened = ImGui::CollapsingHeader(" FLAGS", ImGuiTreeNodeFlags_DefaultOpen);
		ImGui::PopStyleVar(2);
	}

	if (flagsHeaderOpened)
	{
		ImGui::BeginTable("##Flags", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp);
		{
			ImGui::TableSetupColumn("name", 0, 0.3f);
			ImGui::TableSetupColumn("set", 0, 0.6f);
			ImGui::TableSetupColumn("reset", 0, 0.1f);

			ImGui::TableNextColumn();

			{
				ImGui::Text("Culling");
				ImGui::TableNextColumn();

				static const char* cullingType[]{ "Back", "Front", "None"};
				int selectedCulling = static_cast<int>(material->m_CullingType);
				ImGui::Combo("##Culling", &selectedCulling, cullingType, IM_ARRAYSIZE(cullingType));
				material->m_CullingType = static_cast<CullingType>(selectedCulling);

				ImGui::TableNextColumn();
				ImGui::TableNextColumn();
			}

			{
				ImGui::Text("Sampling");
				ImGui::TableNextColumn();

				static const char* samplingType[]{ "Nearest", "Linear" };
				int selectedSampling = static_cast<int>(material->m_SamplingType);
				ImGui::Combo("##Sampling", &selectedSampling, samplingType, IM_ARRAYSIZE(samplingType));
				material->m_SamplingType = static_cast<SamplingType>(selectedSampling);

				ImGui::TableNextColumn();
				ImGui::TableNextColumn();
			}

			{
				ImGui::Text("Receive Shadows");
				ImGui::TableNextColumn();
				
				ImGui::Checkbox("##ReceiveShadows", &material->m_ReceiveShadows);

				ImGui::TableNextColumn();
				ImGui::TableNextColumn();
			}

			{
				ImGui::Text("Cast Shadows");
				ImGui::TableNextColumn();

				ImGui::Checkbox("##CastShadows", &material->m_CastShadows);

				ImGui::TableNextColumn();
				ImGui::TableNextColumn();
			}

			{
				ImGui::Text("Unlit");
				ImGui::TableNextColumn();
				bool val = static_cast<bool>(material->data.u_Unlit);
				ImGui::Checkbox("##Unlit", &val);
				material->data.u_Unlit = static_cast<int>(val);
				ImGui::TableNextColumn();
				ImGui::TableNextColumn();
			}
			
			ImGui::Text("Emissive");
			ImGui::TableNextColumn();

			ImGui::DragFloat("##Emissiveness", &material->data.u_Emissive, 0.1f, 1.0f);
			ImGui::TableNextColumn();
		}
		ImGui::EndTable();
	}

	const auto TexturePanelHeight = 100;
	const ImVec2 TexturePanelSize = ImVec2(0, TexturePanelHeight);
	bool AlbedoOpened;
	{
		UIFont boldfont = UIFont(Fonts::Bold);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 8.f));
		AlbedoOpened = ImGui::CollapsingHeader(" ALBEDO", ImGuiTreeNodeFlags_DefaultOpen);
		ImGui::PopStyleVar(2);
	}

	if (AlbedoOpened)
	{
		ImGui::BeginTable("##Flags", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp);
		{
			ImGui::TableSetupColumn("name", 0, 0.3f);
			ImGui::TableSetupColumn("set", 0, 0.6f);
			ImGui::TableSetupColumn("reset", 0, 0.1f);

			ImGui::TableNextColumn();

			{
				ImGui::Text("Color");
				ImGui::TableNextColumn();

				Vector3 materialColor = material->data.m_AlbedoColor;
				ImVec4 color = { materialColor.r, materialColor.g, materialColor.b, 1.0 };
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::ColorEdit3("##materialAlbedoColor", &material->data.m_AlbedoColor.r, ImGuiColorEditFlags_NoInputs);
				ImGui::PopItemWidth();
				ImGui::TableNextColumn();

				// Reset
				ImGui::TableNextColumn();
			}

			{
				ImGui::Text("Texture");
				ImGui::TableNextColumn();

				ImTextureID textureID = 0;
				if (material->HasAlbedo())
				{
					auto vkTexture = GPUResources::Get().GetTexture(material->AlbedoImage);

					textureID = (ImTextureID)vkTexture->GetImGuiDescriptorSet();
				}

				if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image1"), (ImTextureID)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
				{
					std::string texture = FileDialog::OpenFile("*.png | *.jpg");
					if (texture != "")
					{
						material->SetAlbedo(TextureManager::Get()->GetTexture(texture));
					}
				}

				if (ImGui::BeginPopupContextWindow())
				{
					if (ImGui::MenuItem("Clear Texture"))
					{
						material->m_Albedo = nullptr;
					}
					ImGui::EndPopup();
				}
				ImGui::TableNextColumn();

				// Reset
			}
			
		}
		ImGui::EndTable();
	}

	if (ImGui::CollapsingHeader("Normal", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::BeginTable("##Flags", 3, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingStretchProp);
		{
			ImGui::TableSetupColumn("name", 0, 0.3f);
			ImGui::TableSetupColumn("set", 0, 0.6f);
			ImGui::TableSetupColumn("reset", 0, 0.1f);

			ImGui::TableNextColumn();

			static bool normalEnabled = true;
			{
				ImGui::Text("Enabled");
				ImGui::TableNextColumn();

				ImGui::Checkbox("##normalEnabled", &normalEnabled);
				ImGui::TableNextColumn();

				// Reset
				ImGui::TableNextColumn();
			}

			if (!normalEnabled)
			{
				ImGui::BeginDisabled();
			}

			{
				ImGui::Text("Texture");
				ImGui::TableNextColumn();

				ImTextureID textureID = 0;
				if (material->HasNormal())
				{
					auto vkTexture = GPUResources::Get().GetTexture(material->NormalImage);
					textureID = (ImTextureID)vkTexture->GetImGuiDescriptorSet();
				}

				if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image3"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec4(0, 0, 0, 1), ImVec4(1, 1, 1, 1)))
				{
					std::string texture = FileDialog::OpenFile("*.png | *.jpg");
					if (texture != "")
					{
						material->SetNormal(TextureManager::Get()->GetTexture(texture));
					}
				}

				if (ImGui::BeginPopupContextWindow())
				{
					if (ImGui::MenuItem("Clear Texture"))
					{
						material->m_Normal = nullptr;
					}
					ImGui::EndPopup();
				}
			}

			if (!normalEnabled)
			{
				ImGui::EndDisabled();
			}
		}
		ImGui::EndTable();
	}

	if (ImGui::CollapsingHeader("AO", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::BeginChild("##ao", TexturePanelSize, true);
		{
			ImTextureID textureID = 0;
			if (material->HasAO())
			{
				auto vkTexture = GPUResources::Get().GetTexture(material->AOImage);
				textureID = (ImTextureID)vkTexture->GetImGuiDescriptorSet();
			}

			if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image2"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1)))
			{
				std::string texture = FileDialog::OpenFile("Image files (*.png) | *.png | Image files (*.jpg) | *.jpg");
				if (texture != "")
				{
					material->SetAO(TextureManager::Get()->GetTexture(texture));
				}
			}

			if (ImGui::BeginPopupContextWindow())
			{
				if (ImGui::MenuItem("Clear Texture"))
				{
					material->m_AO = nullptr;
				}
				ImGui::EndPopup();
			}

			ImGui::SameLine();
			ImGui::DragFloat("Value##7", &material->data.u_AOValue, 0.01f, 0.0f, 1.0f);
		}
		ImGui::EndChild();
	}

	if (ImGui::CollapsingHeader("Metalness", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::BeginChild("##metalness", TexturePanelSize, true);
		{
			ImTextureID textureID = 0;
			if (material->HasMetalness())
			{
				auto vkTexture = GPUResources::Get().GetTexture(material->MetalnessImage);
				textureID = (ImTextureID)vkTexture->GetImGuiDescriptorSet();
			}

			if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image4"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec4(0, 0, 0, 1), ImVec4(1, 1, 1, 1)))
			{
				std::string texture = FileDialog::OpenFile("*.png | *.jpg");
				if (texture != "")
				{
					material->SetMetalness(TextureManager::Get()->GetTexture(texture));
				}
			}

			if (ImGui::BeginPopupContextWindow())
			{
				if (ImGui::MenuItem("Clear Texture"))
				{
					material->m_Metalness = nullptr;
				}
				ImGui::EndPopup();
			}

			ImGui::SameLine();
			ImGui::DragFloat("Value##4", &material->data.u_MetalnessValue, 0.01f, 0.0f, 1.0f);
		}
		ImGui::EndChild();
	}

	if (ImGui::CollapsingHeader("Roughness", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::BeginChild("##roughness", TexturePanelSize, true);
		{
			ImTextureID textureID = 0;
			if (material->HasRoughness())
			{
				auto vkTexture = GPUResources::Get().GetTexture(material->RoughnessImage);
				textureID = (ImTextureID)vkTexture->GetImGuiDescriptorSet();
			}

			if (ImGui::ImageButtonEx(ImGui::GetCurrentWindow()->GetID("#image5"), (void*)textureID, ImVec2(80, 80), ImVec2(0, 1), ImVec2(1, 0), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
			{
				std::string texture = FileDialog::OpenFile("*.png | *.jpg");
				if (texture != "")
				{
					material->SetRoughness(TextureManager::Get()->GetTexture(texture));
				}
			}

			if (ImGui::BeginPopupContextWindow())
			{
				if (ImGui::MenuItem("Clear Texture"))
				{
					material->m_Roughness = nullptr;
				}
				ImGui::EndPopup();
			}
			ImGui::SameLine();
			ImGui::DragFloat("Value##6", &material->data.u_RoughnessValue, 0.01f, 0.0f, 1.0f);
		}
		ImGui::EndChild();
	}
}
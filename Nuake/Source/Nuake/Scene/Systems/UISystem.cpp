#include "UISystem.h"

#include "Engine.h"
#include "Nuake/Resource/ResourceManager.h"
#include "Nuake/Resource/ResourceLoader.h"
#include "Nuake/Resource/UI.h"
#include "Nuake/Scene/Scene.h"
#include "Nuake/UI/Nodes/Canvas.h"
#include "Nuake/FileSystem/File.h"
#include "Nuake/Scene/Components/UIComponent.h"
#include "Nuake/FileSystem/FileSystem.h"
#include "Nuake/Rendering/SceneRenderer.h"
#include "Nuake/Core/Input.h"

namespace Nuake
{
	UISystem::UISystem(Scene* scene)
	{
		m_Scene = scene;
	}

	bool UISystem::Init()
	{
		return true;
	}

	void UISystem::Update(Timestep ts)
	{
		auto uiView = m_Scene->m_Registry.view<UIComponent>();
		for (auto e : uiView)
		{
			auto& uiViewComponent = uiView.get<UIComponent>(e);
			Ref<File> file = uiViewComponent.UIFilePath.file;
			if (uiViewComponent.UIResource == 0)
			{
				if (file == nullptr || !file->Exist())
				{
					continue;
				}

				Ref<UIResource> uiResource = ResourceLoader::LoadUI(file);
				uiViewComponent.UIResource = uiResource->ID;
				uis[uiViewComponent.UIResource] = uiResource;

				ResourceManager::RegisterResource(uiResource);
			}
			else
			{
				if (file != nullptr && file->Exist())
				{
					auto ui = ResourceManager::GetResource<UIResource>(uiViewComponent.UIResource);
					bool sourceHasChanged = false;
					if (ui->GetCanvas() == nullptr)
					{
						if (FileSystem::GetFile(uiViewComponent.UIFilePath.file->GetRelativePath())->GetHasBeenModified())
						{
							sourceHasChanged = true;
							FileSystem::GetFile(uiViewComponent.UIFilePath.file->GetRelativePath())->SetHasBeenModified(false);
						}
					}
					else
					{
						for (auto& fileAssociated : ui->GetCanvas()->GetSourceFiles())
						{
							if (!fileAssociated)
							{
								continue;
							}

							// Re-fetching the file object because the Scan might have invalided the pointer.
							if (FileSystem::GetFile(fileAssociated->GetRelativePath())->GetHasBeenModified())
							{
								sourceHasChanged = true;
								FileSystem::GetFile(fileAssociated->GetRelativePath())->SetHasBeenModified(false);
							}
						}
					}

					if (sourceHasChanged)
					{
						uis[uiViewComponent.UIResource]->Reload();
					}

					if (Engine::IsPlayMode())
					{
						// Transform 2D viewport mouse position into project UV position on the UI plane.

						Vector2 screenMousePosition;
						if (Input::IsMouseHidden())
						{
							// Assume we use the middle of the screen
							screenMousePosition = Input::GetViewportSize() / 2.0f;
						}
						else
						{
							screenMousePosition = Input::GetViewportMousePosition();
						}

						// Invert Y
						screenMousePosition.y = m_Scene->m_SceneRenderer->GetGBuffer().GetSize().y - screenMousePosition.y;

						m_Scene->m_SceneRenderer->GetGBuffer().Bind();
						const Vector2& planeUV = m_Scene->m_SceneRenderer->GetGBuffer().ReadVec2(6, screenMousePosition);
						m_Scene->m_SceneRenderer->GetGBuffer().Unbind();

						if (uiViewComponent.IsWorldSpace && planeUV.x > 0 && planeUV.y > 0)
						{
							uis[uiViewComponent.UIResource]->SetMousePosition(planeUV * Vector2{ uiViewComponent.GetResolution().x, uiViewComponent.GetResolution().y });
						}

						uis[uiViewComponent.UIResource]->Tick();
					}

					if (uiViewComponent.IsWorldSpace)
					{
						uis[uiViewComponent.UIResource]->Resize(uiViewComponent.GetResolution());
					}

					uis[uiViewComponent.UIResource]->Draw();
				}
			}
		}
	}

	void UISystem::FixedUpdate(Timestep ts)
	{
	}

	void UISystem::EditorUpdate()
	{
	}

	void UISystem::Exit()
	{
	}
}

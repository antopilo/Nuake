#include "UISystem.h"

#include "Engine.h"
#include "src/Resource/ResourceManager.h"
#include "src/Resource/ResourceLoader.h"
#include "src/Resource/UI.h"
#include "src/Scene/Scene.h"
#include "src/UI/Nodes/Canvas.h"
#include "src/FileSystem/File.h"
#include "src/Scene/Components/UIComponent.h"
#include "src/FileSystem/FileSystem.h"

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

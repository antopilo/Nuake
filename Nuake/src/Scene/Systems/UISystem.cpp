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
			const std::string& filePath = uiViewComponent.UIFilePath;
			if (uiViewComponent.UIResource == 0)
			{
				if (filePath.empty() || !FileSystem::FileExists(filePath))
				{
					continue;
				}

				Ref<UIResource> uiResource = ResourceLoader::LoadUI(filePath);
				uiViewComponent.UIResource = uiResource->ID;
				uis[uiViewComponent.UIResource] = uiResource;

				ResourceManager::RegisterResource(uiResource);
			}
			else
			{
				if (FileSystem::FileExists(filePath))
				{
					auto ui = ResourceManager::GetResource<UIResource>(uiViewComponent.UIResource);
					bool sourceHasChanged = false;
					for (auto& fileAssociated : ui->GetCanvas()->GetSourceFiles())
					{
						// Re-fetching the file object because the Scan might have invalided the pointer.
						if (FileSystem::GetFile(fileAssociated->GetRelativePath())->GetHasBeenModified())
						{
							sourceHasChanged = true;
							FileSystem::GetFile(fileAssociated->GetRelativePath())->SetHasBeenModified(false);
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

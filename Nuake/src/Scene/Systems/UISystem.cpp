#include "UISystem.h"

#include "Engine.h"
#include "src/Resource/ResourceManager.h"
#include "src/Resource/ResourceLoader.h"
#include "src/Resource/UI.h"
#include "src/Scene/Scene.h"
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
					Ref<File> file = FileSystem::GetFile(filePath);
					if (file->GetHasBeenModified())
					{
						uis[uiViewComponent.UIResource]->Reload();
						file->SetHasBeenModified(false);
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

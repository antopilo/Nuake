#pragma once
#include <Nuake/Core/Core.h>
#include "ComponentPanel.h"
#include "ModelResourceInspector.h"
#include "../Misc/PopupHelper.h"

#include <Nuake/Scene/Entities/ImGuiHelper.h>
#include <Nuake/Scene/Components/ModelComponent.h>

#include <Nuake/Resource/ResourceLoader.h>
#include <Nuake/Resource/ResourceManager.h>

#include <Nuake/Core/String.h>
#include <Nuake/Resource/ModelLoader.h>
#include "Nuake/Resource/Bakers/AssetBakerManager.h"
#include "Nuake/Resource/ResourceManager.h"

class MeshPanel : ComponentPanel 
{
	private:
    Scope<ModelResourceInspector> _modelInspector;
    bool _expanded = false;
	
    std::string _importedPathMesh;
	public:
    MeshPanel() 
    {
        CreateScope<ModelResourceInspector>();
    }

    void Draw(Nuake::Entity& entity, entt::meta_any& componentInstance)
    {
        using namespace Nuake;
        
        Nuake::ModelComponent* componentPtr = componentInstance.try_cast<Nuake::ModelComponent>();
        if (componentPtr == nullptr)
        {
            return;
        }
        Nuake::ModelComponent& component = *componentPtr;
        
        BeginComponentTable(MESH, ModelComponent);
        {
            ImGui::Text("Model");
            ImGui::TableNextColumn();

            std::string label = "None";

            const bool isModelNone = component.ModelResource.Get<Model>() == nullptr;
            if (!isModelNone)
            {
                label = std::to_string(component.ModelResource.ID);
            }

            if (ImGui::Button(label.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0)))
            {
                if (!isModelNone)
                {
                    if (!_expanded)
                    {
                        _modelInspector = CreateScope<ModelResourceInspector>(component.ModelResource.Get<Model>());
                    }

                    _expanded = !_expanded;
                }
            }

            if (_expanded)
            {
                _modelInspector->Draw();
            }

            bool shouldConvert = false;
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Model"))
                {
					// Convert payload to relative path
                    std::string fullPath = std::string(static_cast<char*>(payload->Data), 256);
                    fullPath = Nuake::FileSystem::AbsoluteToRelative(fullPath);
                    
					auto file = Nuake::FileSystem::GetFile(fullPath);
					if(file)
					{
						// TODO(antopilo) use file type instead of extension
						if(file->GetExtension() == ".nkmesh")
						{
							auto modelResource = ResourceManager::GetResourceFromFile<Model>(file);
							component.ModelResource = modelResource->ID;
						}
						else
						{
							// Check if we can bake this filetype
							AssetBakerManager& bakerMgr = AssetBakerManager::Get();
							bakerMgr.Bake(file);
						}
						
					}
                }
                ImGui::EndDragDropTarget();
            }
	
            ImGui::TableNextColumn();
            ComponentTableReset(component.ModelPath, "");
        }
        EndComponentTable();
    }
};
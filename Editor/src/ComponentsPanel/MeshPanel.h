#pragma once
#include <src/Core/Core.h>
#include "ComponentPanel.h"
#include "ModelResourceInspector.h"
#include "../Misc/PopupHelper.h"

#include <src/Scene/Entities/ImGuiHelper.h>
#include <src/Scene/Components/ModelComponent.h>

#include <src/Resource/ResourceLoader.h>
#include <src/Resource/ResourceManager.h>

#include <src/Core/String.h>
#include <src/Resource/ModelLoader.h>

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
                    char* file = (char*)payload->Data;
                    std::string fullPath = std::string(file, 256);
                    fullPath = Nuake::FileSystem::AbsoluteToRelative(fullPath);
                    
                    if (Nuake::String::EndsWith(fullPath, ".mesh"))
                    {
                        component.ModelPath = fullPath;
                        //component.ModelResource = ResourceLoader::LoadModel(fullPath);
                    }
                    else
                    {
                        // Convert to .Model
                        //component.ModelPath = fullPath;
                        //component.LoadModel();
                        //
                        //_importedPathMesh = fullPath;
                        //
                        //auto loader = ModelLoader();
                        //auto modelResource = loader.LoadModel(fullPath);
                        //shouldConvert = true;
                    }
                }
                ImGui::EndDragDropTarget();
            }

            if (PopupHelper::DefineConfirmationDialog("##ConvertAsset", "Convert Asset"))
            {
                // Convert to disk
                //auto loader = ModelLoader();
                //Ref<Model> modelResource = loader.LoadModel(_importedPathMesh);
                //json serializedData = modelResource->SerializeData();
                //
                //const std::string exportedMeshPath = _importedPathMesh + ".mesh";
                //FileSystem::BeginWriteFile(exportedMeshPath);
                //FileSystem::WriteLine(serializedData.dump());
                //FileSystem::EndWriteFile();
                //
                //ResourceManager::RegisterResource(modelResource);
                //
                //// Update component
                //component.ModelPath = exportedMeshPath;
                //component.ModelResource = modelResource;
            }

            if (shouldConvert)
            {
                PopupHelper::OpenPopup("##ConvertAsset");
            }

            ImGui::TableNextColumn();
            ComponentTableReset(component.ModelPath, "");
        }
        EndComponentTable();
    }
};
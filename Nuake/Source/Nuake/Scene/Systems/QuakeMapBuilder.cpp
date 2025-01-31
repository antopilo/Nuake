#include "QuakeMapBuilder.h"

#include "Engine.h"
#include "Nuake/Resource/Project.h"

#include "Nuake/Rendering/Mesh/Mesh.h"
#include "Nuake/FileSystem/FileSystem.h"
#include "Nuake/Core/String.h"
#include "Nuake/Scene/Scene.h"
#include "Nuake/Scene/Entities/Entity.h"
#include "Nuake/Scene/Components/QuakeMap.h"
#include "Nuake/Scene/Components/ParentComponent.h"

extern "C" {
    #include <libmap/h/map_parser.h>
    #include <libmap/h/geo_generator.h>
    #include <libmap/h/surface_gatherer.h>
}

#include "Nuake/Rendering/Textures/MaterialManager.h"
#include "Nuake/Scene/Components/BSPBrushComponent.h"
#include "Nuake/Scene/Components/TransformComponent.h"
#include "Nuake/Scene/Components/LightComponent.h"
#include "Nuake/Scene/Components/NameComponent.h"
#include "Nuake/Resource/FGD/FGDClass.h"
#include "Nuake/Scene/Components/PrefabComponent.h"
#include "Nuake/Scene/Components/ModelComponent.h"
#include "Nuake/Resource/ResourceLoader.h"
#include "Nuake/Scene/Components/NetScriptComponent.h"
#include "Nuake/Scripting/ScriptingEngineNet.h"
#include "Nuake/Resource/FGD/FGDFile.h"

#include "Nuake/Resource/ResourceManager.h"

#include <vector>
#include <map>


namespace Nuake {
    struct ProcessedMesh
    {
        std::vector<Vertex> Vertices;
        std::vector<uint32_t> Indices;
    };

    Ref<Material> DefaultMaterial;

    void QuakeMapBuilder::BuildQuakeMap(Entity& ent, bool Collisions)
    {
        if (!ent.HasComponent<QuakeMapComponent>())
            return;

        ent.GetComponent<NameComponent>().IsPrefab = true;

        QuakeMapComponent& quakeMapC = ent.GetComponent<QuakeMapComponent>();
        quakeMapC.m_Brushes.clear();

        ScaleFactor = quakeMapC.ScaleFactor;
        Scene* m_Scene = ent.GetScene();

        // TODO: Tag entities *owned* by the map.
        ParentComponent& currentParent = ent.GetComponent<ParentComponent>();

        // Copy queue, cant delete while iterating.
        auto deletionQueue = currentParent.Children;
        for (auto& e : deletionQueue)
        {
            m_Scene->DestroyEntity(e);
        }

        map_parser_load(quakeMapC.Path.GetAbsolutePath().c_str());
        geo_generator_run();

        DefaultMaterial = MaterialManager::Get()->GetMaterial("default");
        Entity worldspawnEntity = { (entt::entity)-1, m_Scene };
        Entity currentNonWorldEntity;
        std::map<std::string, Entity> pointEntities = std::map<std::string, Entity>();
        for (uint32_t e = 0; e < (uint32_t)entity_count; ++e)
        {
            entity* entity_inst = &entities[e];
            entity_geometry* entity_geo_inst = &entity_geo[e];

            bool isTrigger = false;
            bool isFunc = false;
            bool isPos = false;
            
            std::string target = "";
            std::string targetname = "";
            FGDBrushEntity* fgdBrush;
            FGDPointEntity* pointEntity;
            bool isEntity = false;
            bool isWorldSpawn = false;
            bool isPointEntity = false;
            Vector3 brushLocalPosition = { 0, 0, 0 };
            std::map<std::string, std::string> properties;
            for (int i = 0; i < entity_inst->property_count; i++)
            {
                property* prop = &(entity_inst->properties)[i];

                std::string key = prop->key;
                std::string value = prop->value;
                properties[key] = value;

                if (key == "origin")
                {
                    std::vector<std::string> splits = String::Split(value, ' ');
                    float x = String::ToFloat(splits[1]);
                    float y = String::ToFloat(splits[2]);
                    float z = String::ToFloat(splits[0]);

                    Vector3 position = Vector3(x, y, z) * ScaleFactor * (1.0f / 64.0f);
                    brushLocalPosition = position;
                }

                if (key == "classname")
                {
                    Ref<FGDFile> file = Engine::GetProject()->EntityDefinitionsFile;
                    EntityType type = file->GetTypeOfEntity(value);
                    if (type != EntityType::None)
                    {
                        if (type == EntityType::Brush)
                        {
                            fgdBrush = file->GetBrushEntity(value);

                            if(fgdBrush->Name != "")
                                isEntity = true;
                            else
                                isWorldSpawn = true;
                        }

                        if (type == EntityType::Point)
                        {
                            if (value != "")
                                isPointEntity = true;

                            pointEntity = file->GetPointEntity(value);
                           
                        }
                    }
                    else
                    {
                        isWorldSpawn = true;
                        continue;
                    }
                }
                if (key == "targetname")
                    targetname = value;
                if (key == "target")
                    target = value;
            }

            if (!worldspawnEntity.IsValid())
            {
                worldspawnEntity = m_Scene->CreateEntity("WorldSpawn");
                ent.AddChild(worldspawnEntity);
            }

            if (!isWorldSpawn)
            {
                currentNonWorldEntity = m_Scene->CreateEntity(properties["classname"]);
                ent.AddChild(currentNonWorldEntity);

                auto& transformComponent = currentNonWorldEntity.GetComponent<TransformComponent>();
                transformComponent.SetLocalPosition({entity_inst->center.y / 64.0f, entity_inst->center.z / 64.0f, entity_inst->center.x / 64.0f });
                auto& bsp = currentNonWorldEntity.AddComponent<BSPBrushComponent>();

                if (isPointEntity)
                {
                    std::string name = pointEntity->Name;
                    if (targetname != "")
                    {
                        name = targetname;
                    }

                    bsp.target = target;
                    bsp.TargetName = targetname;

                    if (properties.find("origin") != properties.end())
                    {
                        auto splits = String::Split(properties["origin"], ' ');
                        if (splits.size() == 3)
                        {
                            int x = atoi(splits[0].c_str());
                            int y = atoi(splits[1].c_str());
                            int z = atoi(splits[2].c_str());

                            Vector3 pointPosition = { y / 64.0f, z / 64.0f, x / 64.0f };
                            transformComponent.SetLocalPosition(pointPosition);
                        }
                    }
                    
                    if (pointEntity->Script != "")
                    {
                        NetScriptComponent& netScript = currentNonWorldEntity.AddComponent<NetScriptComponent>();
                        netScript.ScriptPath = pointEntity->Script;

                        ScriptingEngineNet::Get().UpdateEntityWithExposedVar(currentNonWorldEntity);
                        for (auto& ev : netScript.ExposedVar)
                        {
                            if (properties.find(ev.Name) != properties.end())
                            {
                                if (ev.Type == NetScriptExposedVarType::String)
                                {
                                    ev.Value = properties[ev.Name];
                                    ev.DefaultValue = ev.Value;
                                }
                                else if (ev.Type == NetScriptExposedVarType::Float)
                                {
                                    float value = atof(properties[ev.Name].c_str());
                                    ev.Value = value;
                                    ev.DefaultValue = ev.Value;
                                }
                            }
                        }
                    }
                    else if(!pointEntity->Prefab.empty())
                    {
                        //auto& prefabComponent = currentNonWorldEntity.AddComponent<PrefabComponent>();
                        Ref<Prefab> prefab = Prefab::InstanceInScene(pointEntity->Prefab, m_Scene);
                        //prefabComponent.SetPrefab(prefab);
                        //prefabComponent.isInitialized = true;
                        currentNonWorldEntity.AddChild(prefab->Root);
                        //prefab->Root.GetComponent<TransformComponent>().SetLocalPosition(brushLocalPosition);

                        //for (auto& e : prefabComponent.PrefabInstance->Entities)
                        //{
                        //    if (!e.GetComponent<ParentComponent>().HasParent)
                        //    {
                        //        newPrefab.AddChild(e);
                        //    }
                        //}
                    }
                }
                else
                {
                    std::string name = fgdBrush->Name;
                    if (targetname != "")
                    {
                        name = targetname;
                    }

                    bsp.IsSolid = !fgdBrush->IsTrigger;
                    bsp.IsTransparent = !fgdBrush->Visible;
                    bsp.IsFunc = true;
                    bsp.IsTrigger = fgdBrush->IsTrigger;
                    bsp.target = target;
                    bsp.TargetName = targetname;

                    if (fgdBrush->Script != "")
                    {
                        NetScriptComponent& netScript = currentNonWorldEntity.AddComponent<NetScriptComponent>();
                        netScript.ScriptPath = fgdBrush->Script;

                        ScriptingEngineNet::Get().UpdateEntityWithExposedVar(currentNonWorldEntity);
                        for (auto& ev : netScript.ExposedVar)
                        {
                            if (properties.find(ev.Name) != properties.end())
                            {
                                if (ev.Type == NetScriptExposedVarType::String)
                                {
                                    ev.Value = properties[ev.Name];
                                    ev.DefaultValue = ev.Value;
                                }
                            }
                        }
                    }

                    std::map<std::string, Ref<Material>> m_Materials;
                    std::map<Ref<Material>, std::vector<ProcessedMesh>> m_StaticWorld;
                    for (int b = 0; b < entity_inst->brush_count; ++b)
                    {
                        brush* brush_inst = &entity_inst->brushes[b];
                        brush_geometry* brush_geo_inst = &entity_geo_inst->brushes[b];
                        std::vector<Vector3> pointsInBrush;
                        if (isEntity)
                        {
                            std::vector<Vertex> vertices;
                            std::vector<unsigned int> indices;

                            int index_offset = 0;
                            int lastTextureID = -1;
                            std::string lastTexturePath = "";
                            for (int f = 0; f < brush_inst->face_count; ++f)
                            {
                                face* face = &brush_inst->faces[f];
                                texture_data* texture = &textures[face->texture_idx];
                                Ref<Material> currentMaterial;
                                if (std::string(texture->name) != "__TB_empty")
                                {
                                    std::string path = FileSystem::Root + "Textures/" + std::string(texture->name) + ".png";
                                    const std::string materialPath = "Materials/" + std::string(texture->name) + ".material";
                                    if (FileSystem::FileExists(materialPath))
                                    {
                                        Ref<Material> material = ResourceLoader::LoadMaterial(materialPath);
                                        m_Materials[path] = material;
                                    }
                                    else
                                    {
                                        if (m_Materials.find(path) == m_Materials.end())
                                        {
                                            Ref<Material> newMaterial = CreateRef<Material>(path);
                                            auto materialJson = newMaterial->Serialize().dump(4);
                                            FileSystem::BeginWriteFile(materialPath);
                                            FileSystem::WriteLine(materialJson);
                                            FileSystem::EndWriteFile();
                                            ResourceManager::RegisterResource(newMaterial);
                                            ResourceManager::Manifest.RegisterResource(newMaterial->ID, materialPath);
                                            m_Materials[path] = newMaterial;
                                        }
                                    }

                                    currentMaterial = m_Materials[path];
                                    texture->height = currentMaterial->m_Albedo->GetHeight();
                                    texture->width = currentMaterial->m_Albedo->GetWidth();
                                }
                                else
                                {
                                    continue;

                                    currentMaterial = MaterialManager::Get()->GetMaterial("resources/Textures/default/Default.png");
                                    texture->height = texture->width = 1;
                                }

                                face_geometry* face_geo_inst = &brush_geo_inst->faces[f];
                                std::vector<Vertex> vertices;
                                std::vector<uint32_t> indices;
                                for (int i = 0; i < face_geo_inst->vertex_count; ++i)
                                {
                                    face_vertex vertex = face_geo_inst->vertices[i];
                                    vertex_uv vertex_uv;

                                    if (face->is_valve_uv)
                                        vertex_uv = get_valve_uv(vertex.vertex, face, texture->width, texture->height);
                                    else
                                        vertex_uv = get_standard_uv(vertex.vertex, face, texture->width, texture->height);

                                    Vector3 vertexPos = Vector3(
                                        (vertex.vertex.y - entity_inst->center.y) * quakeMapC.ScaleFactor,
                                        (vertex.vertex.z - entity_inst->center.z) * quakeMapC.ScaleFactor,
                                        (vertex.vertex.x - entity_inst->center.x) * quakeMapC.ScaleFactor
                                    );

                                    // We need to push the hull points because the batching
                                    // will create 1 model per material, this prevents us from
                                    // having 1 collision shape per brush(convex).
                                    pointsInBrush.push_back(vertexPos * (1.0f / 64.0f));

                                    Vector2 vertexUV = Vector2(vertex_uv.u, 1.0 - vertex_uv.v);
                                    Vector3 vertexNormal = Vector3(vertex.normal.y, vertex.normal.z, vertex.normal.x);
                                    Vector3 vertexTangent = Vector3(vertex.tangent.y, vertex.tangent.z, vertex.tangent.x);
                                    Vector3 vertexBitangent = glm::cross(vertexNormal, vertexTangent) * (float)vertex.tangent.w;

                                    vertices.push_back(Vertex{
                                        vertexPos * (1.0f / 64.0f),
                                        vertexUV.x,
                                        vertexNormal,
                                        vertexUV.y,
                                        Vector4(vertexTangent, 0),
                                        Vector4(vertexBitangent, 0)
                                    });
                                }

                                for (int i = 0; i < (face_geo_inst->vertex_count - 2) * 3; i += 3)
                                {
                                    uint32_t i1 = face_geo_inst->indices[i];
                                    uint32_t i2 = face_geo_inst->indices[i + 1];
                                    uint32_t i3 = face_geo_inst->indices[i + 2];
                                    indices.push_back(i3);
                                    indices.push_back(i2);
                                    indices.push_back(i1);
                                }

                                m_StaticWorld[currentMaterial].push_back({ vertices, indices });

                                vertices.clear();
                                indices.clear();
                            }

                            bsp.Hulls.push_back(std::move(pointsInBrush));
                        }

                        //else
                        //    CreateBrush(brush_inst, brush_geo_inst, m_Scene, newEntity, target, targetname);
                    }

                    // Batching process`
                    Ref<Model> model = CreateRef<Model>();
                    for (auto& mat : m_StaticWorld)
                    {
                        std::vector<Vertex> batchedVertices;
                        std::vector<uint32_t> batchedIndices;
                        uint32_t indexOffset = 0;
                        for (auto& pm : mat.second)
                        {
                            for (auto& vert : pm.Vertices)
                                batchedVertices.push_back(vert);

                            for (auto& index : pm.Indices)
                                batchedIndices.push_back(indexOffset + index);

                            indexOffset += static_cast<uint32_t>(pm.Vertices.size());
                        }

                        Ref<Mesh> mesh = CreateRef<Mesh>();
                        mesh->AddSurface(batchedVertices, batchedIndices);
                        mesh->SetMaterial(mat.first);
                        model->AddMesh(mesh);
                    }

                    if (!bsp.IsTrigger)
                    {
                        ModelComponent& modelComponent = currentNonWorldEntity.AddComponent<ModelComponent>();
                        //modelComponent.ModelResource = model;
                    }
                }
            }
            else
            {
                std::map<std::string, Ref<Material>> m_Materials;
                std::map<Ref<Material>, std::vector<ProcessedMesh>> m_StaticWorld;

                const std::string entityName = "Brush " + std::to_string(e);
                Entity brushEntity = m_Scene->CreateEntity(entityName);
                worldspawnEntity.AddChild(brushEntity);

                auto& transformComponent = brushEntity.GetComponent<TransformComponent>();
                transformComponent.SetLocalPosition(brushLocalPosition);
                auto& bsp = brushEntity.AddComponent<BSPBrushComponent>();
                
                quakeMapC.m_Brushes.push_back(brushEntity);

                bsp.IsSolid = true;
                bsp.IsTransparent = false;
                bsp.IsFunc = false;
                bsp.IsTrigger = false;
                bsp.target = target;

                for (int b = 0; b < entity_inst->brush_count; ++b)
                {
                    brush* brush_inst = &entity_inst->brushes[b];
                    brush_geometry* brush_geo_inst = &entity_geo_inst->brushes[b];
                    std::vector<Vector3> pointsInBrush;
                    for (int f = 0; f < brush_inst->face_count; ++f)
                    {
                        face* face = &brush_inst->faces[f];
                        texture_data* texture = &textures[face->texture_idx];

                        Ref<Material> currentMaterial;
                        if (std::string(texture->name) != "__TB_empty")
                        {
                            std::string path = FileSystem::Root + "Textures/" + std::string(texture->name) + ".png";
                            const std::string materialPath = "Materials/" + std::string(texture->name) + ".material";
                            if (FileSystem::FileExists(materialPath))
                            {
                                currentMaterial = ResourceManager::GetResourceFromFile<Material>(FileSystem::GetFile(materialPath));
                                m_Materials[path] = currentMaterial;
                            }
                            else
                            {
                                if (m_Materials.find(path) == m_Materials.end())
                                {
                                    Ref<Material> newMaterial = CreateRef<Material>(path);
                                    newMaterial->AlbedoImage = TextureManager::Get()->GetTexture2(path)->GetID();
                                    
                                    auto materialJson = newMaterial->Serialize().dump(4);
                                    FileSystem::BeginWriteFile(materialPath);
                                    FileSystem::WriteLine(materialJson);
                                    FileSystem::EndWriteFile();

                                    FileSystem::Scan();

                                    if (auto file = FileSystem::GetFile(materialPath); file != nullptr)
                                    {
                                        ResourceManager::RegisterUnregisteredRessource(file);
                                    }
                                    else
                                    {
                                        Logger::Log("File at path: " + materialPath + " was not found in internal file system", "FS", CRITICAL);
                                    }

                                    m_Materials[path] = newMaterial;
                                    currentMaterial = newMaterial;
                                }
                            }

                            currentMaterial = m_Materials[path];
                            texture->height = 64;
                            texture->width = 64;
                        }
                        else
                        {
                            continue;

                            currentMaterial = MaterialManager::Get()->GetMaterial("resources/Textures/default/Default.png");
                            texture->height = texture->width = 1;
                        }
                        
                        face_geometry* face_geo_inst = &brush_geo_inst->faces[f];

                        std::vector<Vertex> vertices;
                        std::vector<uint32_t> indices;
                        for (int i = 0; i < face_geo_inst->vertex_count; ++i)
                        {
                            face_vertex vertex = face_geo_inst->vertices[i];
                            vertex_uv vertex_uv;

                            if (face->is_valve_uv)
                                vertex_uv = get_valve_uv(vertex.vertex, face, texture->width, texture->height);
                            else
                                vertex_uv = get_standard_uv(vertex.vertex, face, texture->width, texture->height);

                            Vector3 vertexPos = Vector3(
                                vertex.vertex.y * quakeMapC.ScaleFactor,
                                vertex.vertex.z * quakeMapC.ScaleFactor,
                                vertex.vertex.x * quakeMapC.ScaleFactor
                            );

                            // We need to push the hull points because the batching
                            // will create 1 model per material, this prevents us from
                            // having 1 collision shape per brush(convex).
                            pointsInBrush.push_back(vertexPos * (1.0f / 64.0f));

                            Vector2 vertexUV = Vector2(vertex_uv.u, 1.0 - vertex_uv.v);
                            Vector3 vertexNormal = Vector3(vertex.normal.y, vertex.normal.z, vertex.normal.x);
                            Vector3 vertexTangent = Vector3(vertex.tangent.y, vertex.tangent.z, vertex.tangent.x);
                            Vector3 vertexBitangent = glm::cross(vertexNormal, vertexTangent) * (float)vertex.tangent.w;

                            vertices.push_back(Vertex {
                                vertexPos * (1.0f / 64.0f),
                                vertexUV.x,
                                vertexNormal,
                                vertexUV.y,
                                Vector4(vertexTangent, 0),
                                Vector4(vertexBitangent, 0)
                            });
                        }

                        for (int i = 0; i < (face_geo_inst->vertex_count - 2) * 3; i += 3)
                        {
                            uint32_t i1 = face_geo_inst->indices[i];
                            uint32_t i2 = face_geo_inst->indices[i + 1];
                            uint32_t i3 = face_geo_inst->indices[i + 2];
                            indices.push_back(i3);
                            indices.push_back(i2);
                            indices.push_back(i1);
                        }

                        m_StaticWorld[currentMaterial].push_back({vertices, indices});

                        vertices.clear();
                        indices.clear();
                    }

                    bsp.Hulls.push_back(std::move(pointsInBrush));
                }

                Ref<Model> model = CreateRef<Model>();

                // Batching process
                for (auto& mat : m_StaticWorld)
                {
                    std::vector<Vertex> batchedVertices;
                    std::vector<uint32_t> batchedIndices;
                    uint32_t indexOffset = 0;
                    for (auto& pm : mat.second)
                    {
                        for(auto& vert : pm.Vertices)
                            batchedVertices.push_back(vert);

                        for (auto& index : pm.Indices)
                            batchedIndices.push_back(indexOffset + index);

                        indexOffset += static_cast<uint32_t>(pm.Vertices.size());
                    }

                    Ref<Mesh> mesh = CreateRef<Mesh>();
                    mesh->AddSurface(batchedVertices, batchedIndices);
                    mesh->SetMaterial(mat.first);

                    model->AddMesh(mesh);
                    //bsp.Meshes.push_back(mesh);
                }

                BinarySerializer serializer;
                const std::string assetPath = quakeMapC.Path.GetAbsolutePath() + "." + entityName + ".nkmesh";
                serializer.SerializeModel(assetPath, model);

                ResourceManager::RegisterResource(model);
                ResourceManager::Manifest.RegisterResource(model->ID, FileSystem::AbsoluteToRelative(assetPath));

                ModelComponent& modelComponent = brushEntity.AddComponent<ModelComponent>();
                modelComponent.ModelResource = model->ID;
            }
            
            isEntity = false;
            isPointEntity = false;
        }
    }
}

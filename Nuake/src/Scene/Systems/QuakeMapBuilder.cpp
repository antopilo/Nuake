#include "QuakeMapBuilder.h"

#include "src/Rendering/Mesh/Mesh.h"
#include "src/Core/FileSystem.h"
#include "src/Core/String.h"
#include "src/Scene/Scene.h"
#include "src/Scene/Entities/Entity.h"
#include "src/Scene/Components/QuakeMap.h"
#include "src/Scene/Components/ParentComponent.h"

extern "C" {
    #include <libmap/h/map_parser.h>
    #include <libmap/h/geo_generator.h>
    #include <libmap/h/surface_gatherer.h>
}

#include "src/Rendering/Textures/MaterialManager.h"
#include "src/Scene/Components/BSPBrushComponent.h"
#include "src/Scene/Components/TransformComponent.h"
#include "src/Scene/Components/LightComponent.h"
#include "src/Scene/Components/NameComponent.h"
#include "src/Scene/Components/TriggerZone.h"
#include "src/Resource/FGD/FGDClass.h"
#include "src/Scene/Components/WrenScriptComponent.h"
#include "src/Scene/Components/PrefabComponent.h"
#include "src/Scene/Components/ModelComponent.h"

#include <vector>
#include <map>
#include <src/Resource/ResourceLoader.h>
#include <src/Scene/Components/NetScriptComponent.h>
#include <src/Scripting/ScriptingEngineNet.h>

namespace Nuake {
    struct ProcessedMesh
    {
        std::vector<Vertex> Vertices;
        std::vector<uint32_t> Indices;
    };

    Ref<Material> DefaultMaterial;

    void QuakeMapBuilder::CreateTrigger(brush* brush, brush_geometry* brush_inst, 
                                        Scene* scene, Entity& parent, 
                                        const std::string& target, const std::string& targetname)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        Entity brushEntity = scene->CreateEntity("Trigger");
        TransformComponent& transformComponent = brushEntity.GetComponent<TransformComponent>();
        TriggerZone& trigger = brushEntity.AddComponent<TriggerZone>();
        trigger.target = target;

        parent.AddChild(brushEntity);

        transformComponent.SetGlobalPosition(Vector3(brush->center.y * (1.0f / 64),
            brush->center.z * ScaleFactor * (1.0f / 64),
            brush->center.x * ScaleFactor * (1.0f / 64)));

        BSPBrushComponent& bsp = brushEntity.AddComponent<BSPBrushComponent>();
        bsp.IsSolid = false;
        bsp.target = target;
        int indexOffset = 0;
        for (int f = 0; f < brush->face_count; ++f)
        {
            face* face = &brush->faces[f];

            face_geometry* face_geo_inst = &brush_inst->faces[f];

            for (int i = 0; i < face_geo_inst->vertex_count; ++i)
            {
                face_vertex vertex = face_geo_inst->vertices[i];

                Vector3 vertexPos = Vector3(
                    (vertex.vertex.y - brush->center.y) * ScaleFactor * (1.0f / 64),
                    (vertex.vertex.z - brush->center.z) * ScaleFactor * (1.0f / 64),
                    (vertex.vertex.x - brush->center.x) * ScaleFactor * (1.0f / 64)
                );

                Vector3 vertexNormal = Vector3(vertex.normal.y, vertex.normal.z, vertex.normal.x);
                Vector3 vertexTangent = Vector3(vertex.tangent.y, vertex.tangent.z, vertex.tangent.x);

                vertices.push_back(Vertex{
                    vertexPos,
                    Vector2(0,0),
                    vertexNormal,
                    vertexTangent,
                    glm::vec3(0.0, 1.0, 0.0)
                    });
            }

            for (int i = 0; i < (face_geo_inst->vertex_count - 2) * 3; ++i)
            {
                unsigned int index = face_geo_inst->indices[i];
                indices.push_back((unsigned int)indexOffset + index);
            }

            indexOffset += face_geo_inst->vertex_count;
        }

        Ref<Mesh> mesh = CreateRef<Mesh>();
        mesh->AddSurface(vertices, indices);
        bsp.Meshes.push_back(mesh);
    }

    void QuakeMapBuilder::CreateBrush(brush* brush, brush_geometry* brush_inst, 
                                        Scene* scene, Entity& parent, 
                                        const std::string& target, const std::string& targetname)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        Entity brushEntity = scene->CreateEntity(targetname);
        TransformComponent& transformComponent = brushEntity.GetComponent<TransformComponent>();
        BSPBrushComponent& bsp = brushEntity.AddComponent<BSPBrushComponent>();

        parent.AddChild(brushEntity);

        transformComponent.SetGlobalPosition(Vector3(
            brush->center.y * ScaleFactor * (1.0f / 64),
            brush->center.z * ScaleFactor * (1.0f / 64),
            brush->center.x * ScaleFactor * (1.0f / 64)));

        int index_offset = 0;
        int lastTextureID = -1;
        std::string lastTexturePath = "";
        for (int f = 0; f < brush->face_count; ++f)
        {
            face* face = &brush->faces[f];
            texture_data* texture = &textures[face->texture_idx];

            bool textureIsEmpty = std::string(texture->name) == "__TB_empty" || std::string(texture->name).empty();
            if (textureIsEmpty)
            {
                texture->height = 1;
                texture->width = 1;
                continue;
            }
            else
            {
                if (FileSystem::FileExists("textures/" + std::string(texture->name) + ".material"))
                {

                }
                else
                {
                    std::string path = FileSystem::Root + "textures/" + std::string(texture->name) + ".png";
                    auto tex = TextureManager::Get()->GetTexture(path);

                    texture->height = tex->GetHeight();
                    texture->width = tex->GetWidth();
                }
            }

            face_geometry* face_geo_inst = &brush_inst->faces[f];

            for (int i = 0; i < face_geo_inst->vertex_count; ++i)
            {
                face_vertex vertex = face_geo_inst->vertices[i];
                vertex_uv vertex_uv = get_valve_uv(vertex.vertex, face, texture->width, texture->height);

                Vector3 vertexPos = Vector3(
                    (vertex.vertex.y - brush->center.y) * ScaleFactor * (1.0f / 64),
                    (vertex.vertex.z - brush->center.z) * ScaleFactor * (1.0f / 64),
                    (vertex.vertex.x - brush->center.x) * ScaleFactor * (1.0f / 64)
                );

                Vector2 vertexUV = Vector2(vertex_uv.u, vertex_uv.v);
                Vector3 vertexNormal = Vector3(vertex.normal.y, vertex.normal.z, vertex.normal.x);
                Vector3 vertexTangent = Vector3(vertex.tangent.y, vertex.tangent.z, vertex.tangent.x);

                vertices.push_back(Vertex{
                    vertexPos,
                    vertexUV,
                    vertexNormal,
                    vertexTangent,
                    glm::vec3(0.0, 1.0, 0.0)
                });
            }

            for (int i = 0; i < (face_geo_inst->vertex_count - 2) * 3; ++i)
            {
                unsigned int index = face_geo_inst->indices[i];
                indices.push_back(index_offset + (unsigned int)index);
            }

            if (lastTextureID != face->texture_idx)
            {
                lastTexturePath = FileSystem::Root + "textures/" + std::string(texture->name) + ".png";

                Ref<Mesh> mesh = CreateRef<Mesh>();
                mesh->AddSurface(vertices, indices);

                if (const std::string materialPath = "textures/" + std::string(texture->name) + ".material";
                    FileSystem::FileExists(materialPath))
                {
                    Ref<Material> material = ResourceLoader::LoadMaterial(materialPath);
                    mesh->SetMaterial(material);
                }
                else if (std::string(texture->name) != "__TB_empty")
                {
                    Ref<Material> material = MaterialManager::Get()->GetMaterial(lastTexturePath);
                    mesh->SetMaterial(material);
                }
                
                bsp.Meshes.push_back(mesh);

                index_offset = 0;
                vertices.clear();
                indices.clear();
                lastTextureID = face->texture_idx;
            }
            else
            {
                index_offset += (face_geo_inst->vertex_count);
            }
        }

        if (vertices.size() > 0)
        {
            Ref<Mesh> mesh = CreateRef<Mesh>();
            mesh->AddSurface(vertices, indices);

            Ref<Material> material = MaterialManager::Get()->GetMaterial(lastTexturePath);
            mesh->SetMaterial(material);

            bsp.Meshes.push_back(mesh);
        }
    }

    void QuakeMapBuilder::CreateFuncBrush(brush* brush, brush_geometry* brush_inst, 
                                            Scene* scene, Entity& parent, 
        const std::string& target, const std::string& targetname, FGDBrushEntity fgdBrush, std::map<std::string, std::string> props)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        std::string name = fgdBrush.Name;
        if (targetname != "")
            name = targetname;

        Entity brushEntity = scene->CreateEntity(name);

        TransformComponent& transformComponent = brushEntity.GetComponent<TransformComponent>();
        BSPBrushComponent& bsp = brushEntity.AddComponent<BSPBrushComponent>();

        std::map<std::string, Ref<Material>> m_Materials;

        bsp.IsSolid = !fgdBrush.IsTrigger;
        bsp.IsTransparent = !fgdBrush.Visible;
        bsp.IsFunc = true;
        bsp.IsTrigger = fgdBrush.IsTrigger;
        if (fgdBrush.Script != "")
        {
            NetScriptComponent& netScript = brushEntity.AddComponent<NetScriptComponent>();
            netScript.ScriptPath = fgdBrush.Script;
            
            ScriptingEngineNet::Get().UpdateEntityWithExposedVar(brushEntity);
            for (auto& ev : netScript.ExposedVar)
            {
                if (props.find(ev.Name) != props.end())
                {
                    if (ev.Type == NetScriptExposedVarType::String)
                    {
                        ev.Value = props[ev.Name];
                        ev.DefaultValue = ev.Value;
                    }
                }
            }
        }

        bsp.target = target;
        bsp.TargetName = targetname;

        parent.AddChild(brushEntity);

        transformComponent.SetLocalPosition(Vector3(
            brush->center.y * ScaleFactor * (1.0f / 64),
            brush->center.z * ScaleFactor * (1.0f / 64),
            brush->center.x * ScaleFactor * (1.0f / 64)));

        int index_offset = 0;
        int lastTextureID = -1;
        std::string lastTexturePath = "";
        std::vector<Vector3> pointsInBrush;
        for (int f = 0; f < brush->face_count; ++f)
        {
            face* face = &brush->faces[f];
            texture_data* texture = &textures[face->texture_idx];
            if (std::string(texture->name) == "__TB_empty")
            {
                texture->height = 1;
                texture->width = 1;
            }
            else
            {
                std::string path = FileSystem::Root + std::string(texture->name) + ".png";
                auto tex = TextureManager::Get()->GetTexture(path);
                texture->height = tex->GetHeight();
                texture->width = tex->GetWidth();
            }

            Ref<Material> currentMaterial;
            if (std::string(texture->name) != "__TB_empty")
            {
                std::string path = FileSystem::Root + "Textures/" + std::string(texture->name) + ".png";

                if (const std::string materialPath = "Materials/" + std::string(texture->name) + ".material";
                    FileSystem::FileExists(materialPath))
                {
                    Ref<Material> material = ResourceLoader::LoadMaterial(materialPath);
                    m_Materials[path] = material;
                }
                else
                {
                    if (m_Materials.find(path) == m_Materials.end())
                    {
                        m_Materials[path] = MaterialManager::Get()->GetMaterial(path);
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

            face_geometry* face_geo_inst = &brush_inst->faces[f];

            for (int i = 0; i < face_geo_inst->vertex_count; ++i)
            {
                face_vertex vertex = face_geo_inst->vertices[i];
                vertex_uv vertex_uv = get_valve_uv(vertex.vertex, face, texture->width, texture->height);

                Vector3 vertexPos = Vector3(
                    (vertex.vertex.y - brush->center.y) * ScaleFactor * (1.0f / 64),
                    (vertex.vertex.z - brush->center.z) * ScaleFactor * (1.0f / 64),
                    (vertex.vertex.x - brush->center.x) * ScaleFactor * (1.0f / 64)
                );


                pointsInBrush.push_back(vertexPos);
                Vector2 vertexUV = Vector2(vertex_uv.u, vertex_uv.v);
                Vector3 vertexNormal = Vector3(vertex.normal.y, vertex.normal.z, vertex.normal.x);
                Vector3 vertexTangent = Vector3(vertex.tangent.y, vertex.tangent.z, vertex.tangent.x);

                vertices.push_back(Vertex
                    {
                        vertexPos,
                        vertexUV,
                        vertexNormal,
                        vertexTangent,
                        glm::vec3(0.0, 1.0, 0.0)
                    }
                );
            }

            for (int i = 0; i < (face_geo_inst->vertex_count - 2) * 3; ++i)
            {
                unsigned int index = face_geo_inst->indices[i];
                indices.push_back(index_offset + (unsigned int)index);
            }

            if (!bsp.IsTrigger)
            {
                if (lastTextureID != face->texture_idx)
                {
                    lastTexturePath = FileSystem::Root + "textures/" + std::string(texture->name) + ".png";
                    Ref<Mesh> mesh = CreateRef<Mesh>();
                    mesh->AddSurface(vertices, indices);

                    if (std::string(texture->name) != "__TB_empty")
                    {
                        mesh->SetMaterial(currentMaterial);
                    }

                    bsp.Meshes.push_back(mesh);

                    index_offset = 0;
                    vertices.clear();
                    indices.clear();
                    lastTextureID = face->texture_idx;
                }
                else
                {
                    index_offset += (face_geo_inst->vertex_count);
                }
            }
            else
            {
                index_offset += (face_geo_inst->vertex_count);
            }
        }
        bsp.Hulls.push_back(std::move(pointsInBrush));

        if (bsp.IsTrigger)
        {
            Ref<Mesh> mesh = CreateRef<Mesh>();
            mesh->AddSurface(vertices, indices);

            bsp.Meshes.push_back(mesh);
            vertices.clear();
        }

        if (vertices.size() > 0)
        {
            Ref<Mesh> mesh = CreateRef<Mesh>();
            mesh->AddSurface(vertices, indices);

            Ref<Material> material = MaterialManager::Get()->GetMaterial(lastTexturePath);
            mesh->SetMaterial(material);
            bsp.Meshes.push_back(mesh);

            ModelComponent& modelComponent = brushEntity.AddComponent<ModelComponent>();

            Ref<Model> model = CreateRef<Model>();
            model->AddMesh(mesh);
            modelComponent.ModelResource = model;
        }
            
    }

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

        map_parser_load((FileSystem::Root + quakeMapC.Path).c_str());
        geo_generator_run();

        DefaultMaterial = MaterialManager::Get()->GetMaterial("default");
        Entity worldspawnEntity = { (entt::entity)-1, m_Scene };
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
                if (isPointEntity)
                {
                    if (pointEntities.find(pointEntity->Name) == pointEntities.end())
                    {
                        Entity pointEntityParent = Engine::GetCurrentScene()->CreateEntity(pointEntity->Name);
                        pointEntities[pointEntity->Name] = pointEntityParent;
                        ent.AddChild(pointEntityParent);
                    }

                    Entity newPrefab = Engine::GetCurrentScene()->CreateEntity(pointEntity->Name);

                    pointEntities[pointEntity->Name].AddChild(newPrefab);

                    auto& prefabComponent = newPrefab.AddComponent<PrefabComponent>();
                    prefabComponent.SetPrefab(Prefab::New(pointEntity->Prefab));
                    newPrefab.GetComponent<TransformComponent>().SetLocalPosition(brushLocalPosition);

                    for (auto& e : prefabComponent.PrefabInstance->Entities)
                    {
                        if (!e.GetComponent<ParentComponent>().HasParent)
                        {
                            newPrefab.AddChild(e);
                        }
                    }
                }

                for (int b = 0; b < entity_inst->brush_count; ++b)
                {
                    brush* brush_inst = &entity_inst->brushes[b];
                    brush_geometry* brush_geo_inst = &entity_geo_inst->brushes[b];

                    if (isEntity)
                        CreateFuncBrush(brush_inst, brush_geo_inst, m_Scene, worldspawnEntity, target, targetname, *fgdBrush, properties);
                    //else
                    //    CreateBrush(brush_inst, brush_geo_inst, m_Scene, newEntity, target, targetname);
                }
            }
            else
            {
                std::map<std::string, Ref<Material>> m_Materials;
                std::map<Ref<Material>, std::vector<ProcessedMesh>> m_StaticWorld;

                Entity brushEntity = m_Scene->CreateEntity("Brush " + std::to_string(e));
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

                            if (const std::string materialPath = "Materials/" + std::string(texture->name) + ".material";
                                FileSystem::FileExists(materialPath))
                            {
                                Ref<Material> material = ResourceLoader::LoadMaterial(materialPath);
                                m_Materials[path] = material;
                            }
                            else
                            {
                                if (m_Materials.find(path) == m_Materials.end())
                                {
                                    m_Materials[path] = MaterialManager::Get()->GetMaterial(path);
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
                                vertexUV,
                                vertexNormal,
                                vertexTangent,
                                vertexBitangent
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

                ModelComponent& modelComponent = brushEntity.AddComponent<ModelComponent>();
                modelComponent.ModelResource = model;
            }
            
            isEntity = false;
            isPointEntity = false;
        }
    }
}

#include "QuakeMapBuilder.h"
#include "src/Core/FileSystem.h"
#include "src/Core/String.h"
#include "src/Scene/Scene.h"
#include "src/Scene/Entities/Entity.h"
#include "src/Scene/Components/QuakeMap.h"
#include "src/Scene/Components/ParentComponent.h"
#include <vector>
#include <map>

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

namespace Nuake {
    struct ProcessedMesh
    {
        std::vector<Vertex> Vertices;
        std::vector<unsigned int> Indices;
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

        transformComponent.Translation = Vector3(brush->center.y * (1.0f / 64),
            brush->center.z * ScaleFactor * (1.0f / 64),
            brush->center.x * ScaleFactor * (1.0f / 64));

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
                    glm::vec3(0.0, 1.0, 0.0), 0.0f
                    });
            }

            for (int i = 0; i < (face_geo_inst->vertex_count - 2) * 3; ++i)
            {
                unsigned int index = face_geo_inst->indices[i];
                indices.push_back((unsigned int)indexOffset + index);
            }

            indexOffset += face_geo_inst->vertex_count;
        }
        bsp.Meshes.push_back(CreateRef<Mesh>(vertices, indices, DefaultMaterial));
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

        transformComponent.Translation = Vector3(
            brush->center.y * ScaleFactor * (1.0f / 64),
            brush->center.z * ScaleFactor * (1.0f / 64),
            brush->center.x * ScaleFactor * (1.0f / 64));

        int index_offset = 0;
        int lastTextureID = -1;
        std::string lastTexturePath = "";
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
                std::string path = FileSystem::Root + "textures/" + std::string(texture->name) + ".png";
                auto tex = TextureManager::Get()->GetTexture(path);

                texture->height = tex->GetHeight();
                texture->width = tex->GetWidth();
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
                    glm::vec3(0.0, 1.0, 0.0), 0.0f
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

                if (std::string(texture->name) == "__TB_empty")
                    bsp.Meshes.push_back(CreateRef<Mesh>(vertices, indices, DefaultMaterial));
                else
                    bsp.Meshes.push_back(CreateRef<Mesh>(vertices, indices, MaterialManager::Get()->GetMaterial(lastTexturePath)));

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
            bsp.Meshes.push_back(CreateRef<Mesh>(vertices, indices, MaterialManager::Get()->GetMaterial(lastTexturePath)));
    }

    void QuakeMapBuilder::CreateFuncBrush(brush* brush, brush_geometry* brush_inst, 
                                            Scene* scene, Entity& parent, 
        const std::string& target, const std::string& targetname, FGDBrushEntity fgdBrush)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        std::string name = fgdBrush.Name;
        if (targetname != "")
            name = targetname;

        Entity brushEntity = scene->CreateEntity(name);

        TransformComponent& transformComponent = brushEntity.GetComponent<TransformComponent>();
        BSPBrushComponent& bsp = brushEntity.AddComponent<BSPBrushComponent>();

        bsp.IsSolid = fgdBrush.Solid;
        bsp.IsTransparent = !fgdBrush.Visible;
        bsp.IsFunc = true;
        bsp.IsTrigger = fgdBrush.IsTrigger;
        if (fgdBrush.Script != "" && fgdBrush.Class != "")
        {
            auto& wrenScript = brushEntity.AddComponent<WrenScriptComponent>();
            wrenScript.Script = fgdBrush.Script;
            wrenScript.Class = fgdBrush.Class;
        }

        if (bsp.IsTrigger)
        {
            TriggerZone& trigger = brushEntity.AddComponent<TriggerZone>();
            trigger.target = target;
        }

        bsp.target = target;

        parent.AddChild(brushEntity);

        transformComponent.Translation = Vector3(
            brush->center.y * ScaleFactor * (1.0f / 64),
            brush->center.z * ScaleFactor * (1.0f / 64),
            brush->center.x * ScaleFactor * (1.0f / 64));

        int index_offset = 0;
        int lastTextureID = -1;
        std::string lastTexturePath = "";

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

                vertices.push_back(Vertex
                    {
                        vertexPos,
                        vertexUV,
                        vertexNormal,
                        vertexTangent,
                        glm::vec3(0.0, 1.0, 0.0),
                        0.0f
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
                    if (std::string(texture->name) == "__TB_empty")
                        bsp.Meshes.push_back(CreateRef<Mesh>(vertices, indices, DefaultMaterial));
                    else
                        bsp.Meshes.push_back(CreateRef<Mesh>(vertices, indices, MaterialManager::Get()->GetMaterial(lastTexturePath)));

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

        if (bsp.IsTrigger)
        {
            bsp.Meshes.push_back(CreateRef<Mesh>(vertices, indices, DefaultMaterial));
            vertices.clear();
        }

        if (vertices.size() > 0)
            bsp.Meshes.push_back(CreateRef<Mesh>(vertices, indices, MaterialManager::Get()->GetMaterial(lastTexturePath)));
    }

    void QuakeMapBuilder::BuildQuakeMap(Entity& ent, bool Collisions)
    {
        if (!ent.HasComponent<QuakeMapComponent>())
            return;

        QuakeMapComponent& quakeMapC = ent.GetComponent<QuakeMapComponent>();
        Scene* m_Scene = ent.GetScene();

        // TODO: Tag entities *owned* by the map.
        ParentComponent& currentParent = ent.GetComponent<ParentComponent>();

        // Copy queue, cant delete while iterating.
        auto deletionQueue = currentParent.Children;
        for (auto& e : deletionQueue)
        {
            m_Scene->DestroyEntity(e);
        }

        map_parser_load(std::string(FileSystem::Root + quakeMapC.Path).c_str());
        geo_generator_run();

        DefaultMaterial = MaterialManager::Get()->GetMaterial("resources/Textures/default/Default.png");
        for (int e = 0; e < entity_count; ++e)
        {
            entity* entity_inst = &entities[e];
            entity_geometry* entity_geo_inst = &entity_geo[e];

            Entity newEntity = m_Scene->CreateEntity("Brush " + std::to_string(e));

            if (entity_inst->spawn_type == entity_spawn_type::EST_GROUP)
                newEntity.GetComponent<NameComponent>().Name = "Group " + std::to_string(e);

            bool isTrigger = false;
            bool isFunc = false;
            bool isPos = false;
            ent.AddChild(newEntity);

            std::string target = "";
            std::string targetname = "";
            FGDBrushEntity fgdBrush;
            bool isEntity = false;
            bool isWorldSpawn = false;
            for (int i = 0; i < entity_inst->property_count; i++)
            {
                property* prop = &(entity_inst->properties)[i];
                std::string key = prop->key;
                std::string value = prop->value;

                if (key == "origin")
                {
                    std::vector<std::string> splits = String::Split(value, ' ');
                    float x = String::ToFloat(splits[1]);
                    float y = String::ToFloat(splits[2]);
                    float z = String::ToFloat(splits[0]);

                    Vector3 position = Vector3(x, y, z) * (ScaleFactor * (1.0f / 64.0f));
                    newEntity.GetComponent<TransformComponent>().Translation = position;
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

                            if(fgdBrush.Name != "")
                                isEntity = true;
                            else
                                isWorldSpawn = true;
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
            if (!isWorldSpawn)
            {
                for (int b = 0; b < entity_inst->brush_count; ++b)
                {
                    brush* brush_inst = &entity_inst->brushes[b];
                    brush_geometry* brush_geo_inst = &entity_geo_inst->brushes[b];

                    if (isEntity)
                        CreateFuncBrush(brush_inst, brush_geo_inst, m_Scene, newEntity, target, targetname, fgdBrush);
                    else
                        CreateBrush(brush_inst, brush_geo_inst, m_Scene, newEntity, target, targetname);
                }
            }
            else
            {
                std::map<std::string, Ref<Material>> m_Materials;
                std::map<Ref<Material>, std::vector<ProcessedMesh>> m_StaticWorld;

                Entity brushEntity = m_Scene->CreateEntity("WorldSpawn");
                newEntity.AddChild(brushEntity);

                TransformComponent& transformComponent = brushEntity.GetComponent<TransformComponent>();
                BSPBrushComponent& bsp = brushEntity.AddComponent<BSPBrushComponent>();
                bsp.IsSolid = true;
                bsp.IsTransparent = false;
                bsp.IsFunc = false;
                bsp.IsTrigger = false;
                bsp.target = target;

                for (int b = 0; b < entity_inst->brush_count; ++b)
                {
                    brush* brush_inst = &entity_inst->brushes[b];
                    brush_geometry* brush_geo_inst = &entity_geo_inst->brushes[b];

                    for (int f = 0; f < brush_inst->face_count; ++f)
                    {
                        face* face = &brush_inst->faces[f];
                        texture_data* texture = &textures[face->texture_idx];

                        Ref<Material> currentMaterial;
                        if (std::string(texture->name) != "__TB_empty")
                        {
                            std::string path = FileSystem::Root + "Textures/" + std::string(texture->name) + ".png";
                            if (m_Materials.find(path) == m_Materials.end())
                                m_Materials[path] = MaterialManager::Get()->GetMaterial(path);

                            currentMaterial = m_Materials[path];
                            texture->height = currentMaterial->m_Albedo->GetHeight();
                            texture->width = currentMaterial->m_Albedo->GetWidth();
                        }
                        else
                        {
                            currentMaterial = MaterialManager::Get()->GetMaterial("resources/Textures/default/Default.png");
                            texture->height = texture->width = 1;
                        }
                            

                        face_geometry* face_geo_inst = &brush_geo_inst->faces[f];

                        std::vector<Vertex> vertices;
                        std::vector<unsigned int> indices;
                        for (int i = 0; i < face_geo_inst->vertex_count; ++i)
                        {
                            face_vertex vertex = face_geo_inst->vertices[i];
                            vertex_uv vertex_uv = get_valve_uv(vertex.vertex, face, texture->width, texture->height);

                            Vector3 vertexPos = Vector3(
                                vertex.vertex.y * quakeMapC.ScaleFactor,
                                vertex.vertex.z * quakeMapC.ScaleFactor,
                                vertex.vertex.x * quakeMapC.ScaleFactor
                            );

                            Vector2 vertexUV = Vector2(vertex_uv.u, 1.0 - vertex_uv.v);
                            Vector3 vertexNormal = Vector3(vertex.normal.y, vertex.normal.z, vertex.normal.x);
                            Vector3 vertexTangent = Vector3(vertex.tangent.y, vertex.tangent.z, vertex.tangent.x);
                            Vector3 vertexBitangent = glm::cross(vertexNormal, vertexTangent) * (float)vertex.tangent.w;

                            vertices.push_back(Vertex {
                                vertexPos * (1.0f/64.0f),
                                vertexUV,
                                vertexNormal,
                                vertexTangent,
                                vertexBitangent,
                                0.0f
                            });
                        }

                        for (int i = 0; i < (face_geo_inst->vertex_count - 2) * 3; ++i)
                        {
                            unsigned int index = face_geo_inst->indices[i];
                            indices.push_back((unsigned int)index);
                        }

                        m_StaticWorld[currentMaterial].push_back({vertices, indices});

                        vertices.clear();
                        indices.clear();
                    }
                }

                // Batching process
                for (auto& mat : m_StaticWorld)
                {
                    std::vector<Vertex> batchedVertices;
                    std::vector<unsigned int> batchedIndices;
                    int indexOffset = 0;
                    for (auto& pm : mat.second)
                    {
                        for(auto& vert : pm.Vertices)
                            batchedVertices.push_back(vert);

                        for (auto& index : pm.Indices)
                            batchedIndices.push_back(indexOffset + index);

                        indexOffset += pm.Vertices.size();
                    }

                    bsp.Meshes.push_back(CreateRef<Mesh>(batchedVertices, batchedIndices, mat.first));
                }
            }
            
            isEntity = false;
        }
    }
}

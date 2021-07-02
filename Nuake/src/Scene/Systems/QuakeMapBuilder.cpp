#include "QuakeMapBuilder.h"
#include <src/Scene/Scene.h>
#include <src/Scene/Entities/Entity.h>
#include <src/Scene/Components/QuakeMap.h>
#include <src/Scene/Components/ParentComponent.h>
#include <vector>
#include <iostream>
#include <sstream>
#include "src/Core/FileSystem.h"
extern "C" {
    #include "libmap/h/map_parser.h"
    #include <libmap/h/geo_generator.h>
    #include <libmap/h/surface_gatherer.h>
}

#include <src/Core/MaterialManager.h>
#include <src/Scene/Components/BSPBrushComponent.h>
#include <src/Scene/Components/TransformComponent.h>
#include <src/Scene/Components/LightComponent.h>
#include <src/Scene/Components/NameComponent.h>
#include <src/Scene/Components/TriggerZone.h>

Ref<Material> DefaultMaterial;
std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;

    while (getline(ss, item, delim)) {
        result.push_back(item);
    }

    return result;
}

void QuakeMapBuilder::CreateTrigger(brush* brush, brush_geometry* brush_inst, Scene* scene, Entity& parent, std::string target, std::string targetname)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    Entity brushEntity = scene->CreateEntity("Trigger");
    TransformComponent& transformComponent = brushEntity.GetComponent<TransformComponent>();
    TriggerZone& trigger = brushEntity.AddComponent<TriggerZone>();
    trigger.target = target;

    parent.AddChild(brushEntity);

    transformComponent.Translation = Vector3(brush->center.y * (1.0f / 64),
        brush->center.z * (1.0f / 64),
        brush->center.x * (1.0f / 64));

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
                (vertex.vertex.y - brush->center.y) * (1.0f / 64),
                (vertex.vertex.z - brush->center.z) * (1.0f / 64),
                (vertex.vertex.x - brush->center.x) * (1.0f / 64)
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

void QuakeMapBuilder::CreateBrush(brush* brush, brush_geometry* brush_inst, Scene* scene, Entity& parent, std::string target, std::string targetname)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    Entity brushEntity = scene->CreateEntity(targetname);
    TransformComponent& transformComponent = brushEntity.GetComponent<TransformComponent>();
    BSPBrushComponent& bsp = brushEntity.AddComponent<BSPBrushComponent>();

    parent.AddChild(brushEntity);

    transformComponent.Translation = Vector3(brush->center.y * (1.0f / 64),
        brush->center.z * (1.0f / 64),
        brush->center.x * (1.0f / 64));

    int index_offset = 0;
    int lastTextureID = -1;
    std::string lastTexturePath = "";
    for (int f = 0; f < brush->face_count; ++f)
    {
        face* face = &brush->faces[f];
        texture_data* texture = &textures[face->texture_idx];
        if (std::string(texture->name) == "__TB_empty") {
            texture->height = 1;
            texture->width = 1;
        }
        else {
            std::string path = "resources/Textures/" + std::string(texture->name) + ".png";
            auto tex = TextureManager::Get()->GetTexture(path);
            texture->height = tex->GetHeight();
            texture->width = tex->GetWidth();
        }

        face_geometry* face_geo_inst = &brush_inst->faces[f];

        for (int i = 0; i < face_geo_inst->vertex_count; ++i)
        {
            face_vertex vertex = face_geo_inst->vertices[i];
            vertex_uv vertex_uv = get_standard_uv(vertex.vertex, face, texture->width, texture->height);

            Vector3 vertexPos = Vector3(
                (vertex.vertex.y - brush->center.y) * (1.0f / 64),
                (vertex.vertex.z - brush->center.z) * (1.0f / 64),
                (vertex.vertex.x - brush->center.x) * (1.0f / 64)
            );
            Vector2 vertexUV = Vector2(vertex_uv.u, 1.0 - vertex_uv.v);
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
            lastTexturePath = "resources/Textures/" + std::string(texture->name) + ".png";
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

void QuakeMapBuilder::CreateFuncBrush(brush* brush, brush_geometry* brush_inst, Scene* scene, Entity& parent, std::string target, std::string targetname)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    Entity brushEntity = scene->CreateEntity(targetname);
    TransformComponent& transformComponent = brushEntity.GetComponent<TransformComponent>();
    BSPBrushComponent& bsp = brushEntity.AddComponent<BSPBrushComponent>();
    bsp.IsFunc = true;
    bsp.target = target;

    parent.AddChild(brushEntity);

    transformComponent.Translation = Vector3(brush->center.y * (1.0f / 64),
        brush->center.z * (1.0f / 64),
        brush->center.x * (1.0f / 64));

    int index_offset = 0;
    int lastTextureID = -1;
    std::string lastTexturePath = "";
    for (int f = 0; f < brush->face_count; ++f)
    {
        face* face = &brush->faces[f];
        texture_data* texture = &textures[face->texture_idx];
        if (std::string(texture->name) == "__TB_empty") {
            texture->height = 1;
            texture->width = 1;
        }
        else {
            std::string path = "resources/Textures/" + std::string(texture->name) + ".png";
            auto tex = TextureManager::Get()->GetTexture(path);
            texture->height = tex->GetHeight();
            texture->width = tex->GetWidth();
        }

        face_geometry* face_geo_inst = &brush_inst->faces[f];

        for (int i = 0; i < face_geo_inst->vertex_count; ++i)
        {
            face_vertex vertex = face_geo_inst->vertices[i];
            vertex_uv vertex_uv = get_standard_uv(vertex.vertex, face, texture->width, texture->height);

            Vector3 vertexPos = Vector3(
                (vertex.vertex.y - brush->center.y) * (1.0f / 64),
                (vertex.vertex.z - brush->center.z) * (1.0f / 64),
                (vertex.vertex.x - brush->center.x) * (1.0f / 64)
            );
            Vector2 vertexUV = Vector2(vertex_uv.u, 1.0 - vertex_uv.v);
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
            lastTexturePath = "resources/Textures/" + std::string(texture->name) + ".png";
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
    for (auto& e : deletionQueue) {
        m_Scene->DestroyEntity(e);
    }

    map_parser_load(std::string(FileSystem::Root + quakeMapC.Path).c_str());
    geo_generator_run();

    DefaultMaterial = MaterialManager::Get()->GetMaterial("resources/Textures/default/Default.png");
    for (int e = 0; e < entity_count; ++e)
    {
        entity* entity_inst = &entities[e];
        entity_geometry* entity_geo_inst = &entity_geo[e];

        Entity newEntity = m_Scene->CreateEntity("Brush " + std::to_string(e) );

        if (entity_inst->spawn_type == entity_spawn_type::EST_GROUP)
            newEntity.GetComponent<NameComponent>().Name = "Group " + std::to_string(e);

        bool isTrigger = false;
        bool isFunc = false;
        bool isPos = false;
        ent.AddChild(newEntity);

        std::string target = "";
        std::string targetname = "";
        for (int i = 0; i < entity_inst->property_count; i++) 
        {
            property* prop = &(entity_inst->properties)[i];
            std::string key = prop->key;
            std::string value = prop->value;

            if (key == "origin") 
            {
                // Position
                std::vector<std::string> splits = split(value, ' ');

                float x = std::stof(splits[1]) * (1.f / 64.f);
                float y = std::stof(splits[2]) * (1.f / 64.f);
                float z = std::stof(splits[0]) * (1.f / 64.f);

                Vector3 position = Vector3(x, y, z);
                newEntity.GetComponent<TransformComponent>().Translation = position;
            }

            if (key == "classname") 
            {
                if (value == "light") 
                {
                    newEntity.AddComponent<LightComponent>();
                    newEntity.GetComponent<NameComponent>().Name = "Light " + std::to_string(i);
                }
                else if (value == "trigger") 
                {
                    //newEntity.AddComponent<LightComponent>();
                    isTrigger = true;
                    newEntity.GetComponent<NameComponent>().Name = "Trigger " + std::to_string(i);
                }
                else if (value == "func_any") 
                {
                    isFunc = true;
                    newEntity.GetComponent<NameComponent>().Name = "func_any " + std::to_string(i);
                }
                else if (value == "position") 
                {
                    isPos = true;
                }
            }
            if (key == "targetname") {
                if (isPos) {

                    newEntity.GetComponent<NameComponent>().Name = value;
                    isPos = false;
                }
                targetname = value;
            }
            if (key == "target") {
                target = value;
            }
        }
        for (int b = 0; b < entity_inst->brush_count; ++b)
        {
            brush* brush_inst = &entity_inst->brushes[b];
            brush_geometry* brush_geo_inst = &entity_geo_inst->brushes[b];

            if (isTrigger) {
                CreateTrigger(brush_inst, brush_geo_inst, m_Scene, newEntity, target, targetname);
            }
            else if (isFunc) {
                CreateFuncBrush(brush_inst, brush_geo_inst, m_Scene, newEntity, target, targetname);
            }
            else {
                CreateBrush(brush_inst, brush_geo_inst, m_Scene, newEntity, target, targetname);
            }
        }
    }
}
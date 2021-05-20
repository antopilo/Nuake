#include "QuakeMap.h"
#include "../Core/Core.h"
#include "../Core/MaterialManager.h"
// Lib map stuff.
extern "C" {
#include "libmap/h/map_parser.h"
#include <libmap/h/geo_generator.h>
#include <libmap/h/surface_gatherer.h>
}

void QuakeMapComponent::Draw()
{
    for (auto m : m_Meshes) {
        m->Draw();
    }
}



void QuakeMapComponent::Load(std::string path, bool collisions)
{
	if (Path == path)
		return;

	Path = path;

    Build();
}

void QuakeMapComponent::Build()
{
    m_Meshes.clear();
    map_parser_load(Path.c_str());

    geo_generator_run();

    Ref<Material> DefaultMaterial = MaterialManager::Get()->GetMaterial("resources/Textures/default/Default.png");
    for (int e = 0; e < entity_count; ++e)
    {
        entity* entity_inst = &entities[e];
        entity_geometry* entity_geo_inst = &entity_geo[e];


        for (int b = 0; b < entity_inst->brush_count; ++b)
        {
            brush* brush_inst = &entity_inst->brushes[b];
            brush_geometry* brush_geo_inst = &entity_geo_inst->brushes[b];

            std::vector<Vertex> vertices;
            std::vector<unsigned int> indices;

            int index_offset = 0;
            int lastTextureID = -1;
            std::string lastTexturePath = "";
            for (int f = 0; f < brush_inst->face_count; ++f)
            {
                face* face = &brush_inst->faces[f];
                texture_data* texture = &textures[face->texture_idx];
                if (std::string(texture->name) == "__TB_empty")
                {
                    texture->height = 1;
                    texture->width = 1;
                }
                else
                {
                    std::string path = "resources/Textures/" + std::string(texture->name) + ".png";
                    auto tex = TextureManager::Get()->GetTexture(path);
                    texture->height = tex->GetHeight();
                    texture->width = tex->GetWidth();
                }


                face_geometry* face_geo_inst = &brush_geo_inst->faces[f];
                //printf("Face %d\n", f);
                for (int i = 0; i < face_geo_inst->vertex_count; ++i)
                {
                    face_vertex vertex = face_geo_inst->vertices[i];
                    vertex_uv vertex_uv = get_standard_uv(vertex.vertex, face, texture->width, texture->height);
                    vertices.push_back(Vertex{
                        glm::vec3(vertex.vertex.y * (1.0f / 64), vertex.vertex.z * (1.0f / 64), vertex.vertex.x * (1.0f / 64)),
                        glm::vec2(vertex_uv.u, vertex_uv.v),
                        glm::vec3(vertex.normal.x, vertex.normal.z, vertex.normal.y),
                        glm::vec3(vertex.tangent.x, vertex.tangent.z, vertex.tangent.y), glm::vec3(0.0, 1.0, 0.0), 0.0f
                        });

                    //printf("vertex: (%f %f %f), normal: (%f %f %f)\n",
                    //    vertex.vertex.x, vertex.vertex.y, vertex.vertex.z,
                    //    vertex.normal.x, vertex.normal.y, vertex.normal.z);
                }

                //puts("Indices:");
                for (int i = 0; i < (face_geo_inst->vertex_count - 2) * 3; ++i)
                {
                    unsigned int index = face_geo_inst->indices[i];
                    //printf("index: %d\n", index_offset + index);
                    indices.push_back(index_offset + (unsigned int)index);
                }
                if (lastTextureID != face->texture_idx)
                {
                    lastTexturePath = "resources/Textures/" + std::string(texture->name) + ".png";
                    if (std::string(texture->name) == "__TB_empty")
                        m_Meshes.push_back(CreateRef<Mesh>(vertices, indices, DefaultMaterial));
                    else
                        m_Meshes.push_back(CreateRef<Mesh>(vertices, indices, MaterialManager::Get()->GetMaterial(lastTexturePath)));



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
                m_Meshes.push_back(CreateRef<Mesh>(vertices, indices, MaterialManager::Get()->GetMaterial(lastTexturePath)));
            //putchar('\n');
            //putchar('\n');
        }
    }
}
void QuakeMapComponent::DrawEditor()
{

}
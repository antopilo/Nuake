#include "../h/surface_gatherer.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "../h/map_data.h"
#include "../h/face.h"
#include "../h/brush.h"
#include "../h/entity.h"

enum surface_split_type split_type = SST_NONE;
int entity_filter_idx = -1;
int texture_filter_idx = -1;
int brush_filter_texture_idx;
int face_filter_texture_idx;
bool filter_worldspawn_layers;

surfaces out_surfaces;

void surface_gatherer_set_split_type(enum surface_split_type new_split_type)
{
    split_type = new_split_type;
}

void surface_gatherer_set_entity_index_filter(int entity_idx)
{
    entity_filter_idx = entity_idx;
}

void surface_gatherer_set_texture_filter(const char *texture_name)
{
    texture_filter_idx = map_data_find_texture(texture_name);
}

void surface_gatherer_set_brush_filter_texture(const char *texture_name)
{
    brush_filter_texture_idx = map_data_find_texture(texture_name);
}

void surface_gatherer_set_face_filter_texture(const char *texture_name)
{
    face_filter_texture_idx = map_data_find_texture(texture_name);
}

void surface_gatherer_set_worldspawn_layer_filter(bool filter)
{
    filter_worldspawn_layers = filter;
}

bool surface_gatherer_filter_entity(int entity_idx)
{
    const entity *ents = map_data_get_entities();
    const entity *ent = &ents[entity_idx];

    // Omit filtered entity indices
    if(entity_filter_idx != -1 && entity_idx != entity_filter_idx)
    {
        return true;
    }

    return false;
}

bool surface_gatherer_filter_brush(int entity_idx, int brush_idx)
{
    const entity *ents = map_data_get_entities();
    brush *brush_inst = &ents[entity_idx].brushes[brush_idx];

    // Omit brushes that are fully-textured with clip
    if (brush_filter_texture_idx != -1)
    {
        bool fully_textured = true;

        for (int f = 0; f < brush_inst->face_count; ++f)
        {
            face *face_inst = &brush_inst->faces[f];
            if (face_inst->texture_idx != brush_filter_texture_idx)
            {
                fully_textured = false;
                break;
            }
        }
        
        if(fully_textured)
        {
            return true;
        }
    }

    // Omit brushes that are part of a worldspawn layer
    for (int f = 0; f < brush_inst->face_count; ++f)
    {
        face *face_inst = &brush_inst->faces[f];
        for (int l = 0; l < worldspawn_layer_count; ++l)
        {
            worldspawn_layer *layer = &worldspawn_layers[l];
            if (face_inst->texture_idx == layer->texture_idx)
            {
                return filter_worldspawn_layers;
            }
            
        }
    }
    
    return false;
}

bool surface_gatherer_filter_face(int entity_idx, int brush_idx, int face_idx)
{
    const entity *ents = map_data_get_entities();
    face *face_inst = &ents[entity_idx].brushes[brush_idx].faces[face_idx];
    face_geometry *face_geo_inst = &entity_geo[entity_idx].brushes[brush_idx].faces[face_idx];

    // Omit faces with less than 3 vertices
    if (face_geo_inst->vertex_count < 3)
    {
        return true;
    }

    // Omit faces that are textured with skip
    if (face_filter_texture_idx != -1 && face_inst->texture_idx == face_filter_texture_idx)
    {
        return true;
    }

    // Omit filtered texture indices
    if (texture_filter_idx != -1 && face_inst->texture_idx != texture_filter_idx)
    {
        return true;
    }

    return false;
}

void surface_gatherer_reset_state()
{
    for (int s = 0; s < out_surfaces.surface_count; ++s)
    {
        surface *surf = &out_surfaces.surfaces[s];
        if (surf->vertices != NULL)
        {
            free(surf->vertices);
            surf->vertices = NULL;
        }

        if (surf->indices != NULL)
        {
            free(surf->indices);
            surf->indices = NULL;
        }
    }

    if (out_surfaces.surfaces != NULL)
    {
        free(out_surfaces.surfaces);
        out_surfaces.surfaces = NULL;
    }

    out_surfaces.surface_count = 0;
}

void surface_gatherer_run()
{
    surface_gatherer_reset_state();

    int index_offset = 0;
    surface *surf_inst = NULL;

    if (split_type == SST_NONE)
    {
        index_offset = 0;
        surf_inst = surface_gatherer_add_surface();
    }

    for (int e = 0; e < entity_count; ++e)
    {
        const entity *entity_inst = &entities[e];
        const entity_geometry *entity_geo_inst = &entity_geo[e];

        if (surface_gatherer_filter_entity(e))
        {
            continue;
        }

        if (split_type == SST_ENTITY)
        {
            if(entity_inst->spawn_type == EST_MERGE_WORLDSPAWN)
            {
                surface_gatherer_add_surface();
                surf_inst = &out_surfaces.surfaces[0];
                index_offset = surf_inst->vertex_count;
            }
            else
            {
                surf_inst = surface_gatherer_add_surface();
                index_offset = surf_inst->vertex_count;
            }
        }

        for (int b = 0; b < entity_inst->brush_count; ++b)
        {
            brush *brush_inst = &entity_inst->brushes[b];
            brush_geometry *brush_geo_inst = &entity_geo_inst->brushes[b];

            if (surface_gatherer_filter_brush(e, b))
            {
                continue;
            }

            if (split_type == SST_BRUSH)
            {
                index_offset = 0;
                surf_inst = surface_gatherer_add_surface();
            }

            for (int f = 0; f < brush_inst->face_count; ++f)
            {
                face_geometry *face_geo_inst = &brush_geo_inst->faces[f];

                if (surface_gatherer_filter_face(e, b, f))
                {
                    continue;
                }

                for (int v = 0; v < face_geo_inst->vertex_count; ++v)
                {
                    face_vertex vertex = face_geo_inst->vertices[v];

                    if(entity_inst->spawn_type == EST_ENTITY || entity_inst->spawn_type == EST_GROUP)
                    {
                        vertex.vertex = vec3_sub(vertex.vertex, entity_inst->center);
                    }

                    surf_inst->vertices = realloc(surf_inst->vertices, (surf_inst->vertex_count + 1) * sizeof(face_vertex));
                    surf_inst->vertices[surf_inst->vertex_count] = vertex;
                    surf_inst->vertex_count++;
                }

                for (int i = 0; i < (face_geo_inst->vertex_count - 2) * 3; ++i)
                {
                    surf_inst->indices = realloc(surf_inst->indices, (surf_inst->index_count + 1) * sizeof(int));
                    surf_inst->indices[surf_inst->index_count] = face_geo_inst->indices[i] + index_offset;
                    surf_inst->index_count++;
                }

                index_offset += face_geo_inst->vertex_count;
            }
        }
    }
}

const surfaces *surface_gatherer_fetch()
{
    return &out_surfaces;
}

surface *surface_gatherer_add_surface()
{
    out_surfaces.surfaces = realloc(out_surfaces.surfaces, (out_surfaces.surface_count + 1) * sizeof(surface));
    surface *surf_inst = &out_surfaces.surfaces[out_surfaces.surface_count];
    *surf_inst = (surface){0};
    out_surfaces.surface_count++;

    return surf_inst;
}

void surface_gatherer_reset_params()
{
    split_type = SST_NONE;
    entity_filter_idx = -1;
    texture_filter_idx = -1;
    brush_filter_texture_idx = -1;
    face_filter_texture_idx = -1;
    filter_worldspawn_layers = true;
}

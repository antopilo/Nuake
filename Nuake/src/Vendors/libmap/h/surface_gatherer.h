#ifndef SURFACE_GATHERER_H
#define SURFACE_GATHERER_H

#include <stdint.h>

#include "libmap.h"
#include "entity_geometry.h"

typedef struct surface {
    int vertex_count;
    face_vertex *vertices;
    int index_count;
    int *indices;
} surface;

typedef struct surfaces {
    int surface_count;
    surface *surfaces;
} surfaces;

enum surface_split_type
{
    SST_NONE,
    SST_ENTITY,
    SST_BRUSH
};

extern LIBMAP_API void surface_gatherer_set_split_type(enum surface_split_type split_type);
extern LIBMAP_API void surface_gatherer_set_brush_filter_texture(const char *texture_name);
extern LIBMAP_API void surface_gatherer_set_face_filter_texture(const char *texture_name);
extern LIBMAP_API void surface_gatherer_set_entity_index_filter(int entity_idx);
extern LIBMAP_API void surface_gatherer_set_texture_filter(const char *texture_name);
extern LIBMAP_API void surface_gatherer_set_worldspawn_layer_filter(bool filter);
extern LIBMAP_API void surface_gatherer_run();
extern LIBMAP_API const surfaces* surface_gatherer_fetch();

bool surface_gatherer_filter_entity(int entity_idx);
bool surface_gatherer_filter_brush(int entity_idx, int brush_idx);
bool surface_gatherer_filter_face(int entity_idx, int brush_idx, int face_idx);

surface* surface_gatherer_add_surface();
void surface_gatherer_reset_state();
extern LIBMAP_API void surface_gatherer_reset_params();

#endif

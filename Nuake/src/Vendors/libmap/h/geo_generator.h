#ifndef GEO_GENERATOR_H
#define GEO_GENERATOR_H

#include "map_data.h"
#include "libmap.h"
#include "vector.h"
#include "brush.h"

extern LIBMAP_API void geo_generator_run();

void generate_brush_vertices(int entity_idx, int brush_idx);
bool intersect_faces(face f0, face f1, face f2, vec3 *o_vertex);
bool vertex_in_hull(face *faces, int face_count, vec3 vertex);

vertex_uv get_standard_uv(vec3 vertex, const face* face, int texture_width, int texture_height);
vertex_uv get_valve_uv(vec3 vertex, const face* face, int texture_width, int texture_height);

vertex_tangent get_standard_tangent(const face* face);
vertex_tangent get_valve_tangent(const face* face);

extern LIBMAP_API void geo_generator_print_entities();
extern LIBMAP_API const entity_geometry* geo_generator_get_entities();
extern LIBMAP_API int geo_generator_get_brush_vertex_count(int entity_idx, int brush_idx);
extern LIBMAP_API int geo_generator_get_brush_index_count(int entity_idx, int brush_idx);
extern LIBMAP_API void geo_generator_get_brush_vertices(int entity_idx, int brush_idx, face_vertex *o_vertices, int *o_indices);

#endif

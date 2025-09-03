#ifndef ENTITY_GEOMETRY_H
#define ENTITY_GEOMETRY_H

#include "vector.h"

typedef struct vertex_uv {
    double u;
    double v;
} vertex_uv;

typedef struct vertex_tangent {
    double x;
    double y;
    double z;
    double w;
} vertex_tangent;

typedef struct face_vertex {
    vec3 vertex;
    vec3 normal;
    vertex_uv uv;
    vertex_tangent tangent;
} face_vertex;

typedef struct face_geometry {
    int vertex_count;
    face_vertex *vertices;
    int index_count;
    int *indices;
} face_geometry;

typedef struct brush_geometry {
    face_geometry * faces;
} brush_geometry;

typedef struct entity_geometry {
    brush_geometry *brushes;
} entity_geometry;

#endif

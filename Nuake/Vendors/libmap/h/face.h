#ifndef FACE_H
#define FACE_H

#include "stdbool.h"
#include "vector.h"

typedef struct face_points
{
    vec3 v0;
    vec3 v1;
    vec3 v2;
} face_points;

typedef struct standard_uv {
    double u;
    double v;
} standard_uv;

typedef struct valve_texture_axis {
    vec3 axis;
    double offset;
} valve_texture_axis;

typedef struct valve_uv {
    valve_texture_axis u;
    valve_texture_axis v;
} valve_uv;

typedef struct face_uv_extra
{
    double rot;
    double scale_x;
    double scale_y;
} face_uv_extra;

typedef struct face
{
    face_points plane_points;
    vec3 plane_normal;
    double plane_dist;

    int texture_idx;

    bool is_valve_uv;
    standard_uv uv_standard;
    valve_uv uv_valve;
    face_uv_extra uv_extra;
} face;

#endif
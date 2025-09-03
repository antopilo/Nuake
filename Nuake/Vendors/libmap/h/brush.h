#ifndef BRUSH_H
#define BRUSH_H

typedef struct face face;

typedef struct brush {
    int face_count;
    face *faces;
    vec3 center;
} brush;

#endif
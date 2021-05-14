#ifndef ENTITY_H
#define ENTITY_H

#include "vector.h"

typedef struct brush brush;

typedef struct property {
    char* key;
    char* value;
} property;

enum entity_spawn_type
{
    EST_WORLDSPAWN = 0,
    EST_MERGE_WORLDSPAWN = 1,
    EST_ENTITY = 2,
    EST_GROUP = 3
};

typedef struct entity {
    int property_count;
    property *properties;
    
    int brush_count;
    brush *brushes;

    vec3 center;
    enum entity_spawn_type spawn_type;
} entity;

#endif
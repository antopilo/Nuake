

#include "../h/map_parser.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "../h/platform.h"
#include "../h/face.h"
#include "../h/brush.h"
#include "../h/entity.h"
#include "../h/map_data.h"

#define DEBUG false


typedef enum parse_scope
{
    PS_FILE,
    PS_COMMENT,
    PS_ENTITY,
    PS_PROPERTY_VALUE,
    PS_BRUSH,
    PS_PLANE_0,
    PS_PLANE_1,
    PS_PLANE_2,
    PS_TEXTURE,
    PS_U,
    PS_V,
    PS_VALVE_U,
    PS_VALVE_V,
    PS_ROT,
    PS_U_SCALE,
    PS_V_SCALE,
} parse_scope;

static parse_scope scope = PS_FILE;
static bool comment = false;
static int entity_idx = -1;
static int brush_idx = -1;
static int face_idx = -1;
static int component_idx = 0;
static char *current_property;
static bool valve_uvs = false;

static face current_face;
static brush current_brush;
static entity current_entity;

void reset_current_face()
{
    current_face = (face){0};
}

void reset_current_brush()
{
    if (current_brush.faces != NULL)
    {
        free(current_brush.faces);
        current_brush.faces = NULL;
    }

    current_brush.face_count = 0;
}

void reset_current_entity()
{
    for (int i = 0; i < current_entity.brush_count; ++i)
    {
        if (current_entity.brushes[i].faces != NULL)
        {
            free(current_entity.brushes[i].faces);
            current_entity.brushes[i].faces = NULL;
        }
    }

    if (current_entity.properties != NULL)
    {
        for (int p = 0; p < current_entity.property_count; ++p)
        {
            if (current_entity.properties[p].key != NULL)
            {
                free(current_entity.properties[p].key);
            }

            if (current_entity.properties[p].value != NULL)
            {
                free(current_entity.properties[p].value);
            }
        }

        free(current_entity.properties);
        current_entity.properties = NULL;
    }

    current_entity.property_count = 0;

    if (current_entity.brushes != NULL)
    {
        free(current_entity.brushes);
        current_entity.brushes = NULL;
    }

    current_entity.brush_count = 0;
}

bool map_parser_load(const char *map_file)
{
    map_data_reset();

    reset_current_face();
    reset_current_brush();
    reset_current_entity();

    scope = PS_FILE;
    comment = false;
    entity_idx = -1;
    brush_idx = -1;
    face_idx = -1;
    component_idx = 0;
    valve_uvs = false;

    FILE *map = fopen(map_file, "r");

    if (!map)
    {
        printf("Error: Failed to open map file.\n");
        return false;
    }

    int c;
    char buf[255];
    int buf_head = 0;
    while ((c = fgetc(map)) != EOF)
    {
        if (c == '\n')
        {
            buf[buf_head] = '\0';
            token(buf);
            buf_head = 0;

            newline();
        }
        else if (isspace(c))
        {
            buf[buf_head] = '\0';
            token(buf);
            buf_head = 0;
        }
        else
        {
            buf[buf_head++] = c;
        }
    }

    fclose(map);

    return true;
}

void set_scope(parse_scope new_scope)
{
#if DEBUG
    switch (new_scope)
    {
    case PS_FILE:
        puts("Switching to file scope\n");
        break;
    case PS_ENTITY:
        printf("Switching to entity %d scope\n", entity_idx);
        break;
    case PS_PROPERTY_VALUE:
        puts("Switching to property value scope\n");
        break;
    case PS_BRUSH:
        printf("Switching to brush %d scope\n", brush_idx);
        break;
    case PS_PLANE_0:
        printf("Switching to face %d plane 0 scope\n", face_idx);
        break;
    case PS_PLANE_1:
        printf("Switching to face %d plane 1 scope\n", face_idx);
        break;
    case PS_PLANE_2:
        printf("Switching to face %d plane 2 scope\n", face_idx);
        break;
    case PS_TEXTURE:
        puts("Switching to texture scope\n");
        break;
    case PS_U:
        puts("Switching to U scope\n");
        break;
    case PS_V:
        puts("Switching to V scope\n");
        break;
    case PS_VALVE_U:
        puts("Switching to Valve U scope\n");
        break;
    case PS_VALVE_V:
        puts("Switching to Valve V scope\n");
        break;
    case PS_ROT:
        puts("Switching to rotation scope\n");
        break;
    case PS_U_SCALE:
        puts("Switching to U scale scope\n");
        break;
    case PS_V_SCALE:
        puts("Switching to V scale scope\n");
        break;
    }

#endif
    scope = new_scope;
}

bool strings_match(const char *lhs, const char *rhs)
{
    return strcmp(lhs, rhs) == 0;
}

void token(const char *buf)
{
    property *prop = NULL;

    if (comment)
    {
        return;
    }
    else if (strings_match(buf, "//"))
    {
        comment = true;
        return;
    }

#if DEBUG
    puts(buf);
#endif

    switch (scope)
    {
    case PS_FILE:
        if (strings_match(buf, "{"))
        {
            entity_idx++;
            brush_idx = -1;
            set_scope(PS_ENTITY);
        }
        break;
    case PS_ENTITY:
        if (buf[0] == '"')
        {
            current_entity.properties = realloc(current_entity.properties, (current_entity.property_count + 1) * sizeof(property));
            prop = &current_entity.properties[current_entity.property_count];
            *prop = (property){0};
            prop->key = STRDUP(&buf[1]);

            size_t last = strlen(prop->key) - 1;
            if (prop->key[last] == '"')
            {
                prop->key[strlen(prop->key) - 1] = '\0';
                set_scope(PS_PROPERTY_VALUE);
            }
        }
        else if (strings_match(buf, "{"))
        {
            brush_idx++;
            face_idx = -1;
            set_scope(PS_BRUSH);
        }
        else if (strings_match(buf, "}"))
        {
            commit_entity(&current_entity);
            set_scope(PS_FILE);
        }
        break;
    case PS_PROPERTY_VALUE:
        prop = &current_entity.properties[current_entity.property_count];

        size_t buf_length = strlen(buf);

        bool is_first = buf[0] == '"';
        bool is_last = buf[buf_length - 1] == '"';

        if (is_first)
        {
            if(current_property != NULL)
            {
                free(current_property);
                current_property = NULL;
            }
        }

        size_t current_length = 0;
        if(current_property != NULL)
        {
            current_length = strlen(current_property);
        };

        if(is_first || is_last)
        {
            current_property = realloc(current_property, current_length + buf_length + 1);
            memcpy(&current_property[current_length], buf, buf_length + 1);
        }
        else
        {
            current_property = realloc(current_property, current_length + buf_length + 3);
            current_property[current_length] = ' ';
            memcpy(&current_property[current_length + 1], buf, buf_length);
            current_property[current_length + buf_length + 1] = ' ';
            current_property[current_length + buf_length + 2] = '\0';
        }

        if (is_last)
        {
            prop->value = STRDUP(&current_property[1]);
            prop->value[strlen(prop->value) - 1] = '\0';
            current_entity.property_count++;
            set_scope(PS_ENTITY);
        }
        break;
    case PS_BRUSH:
        if (strings_match(buf, "("))
        {
            face_idx++;
            component_idx = 0;
            set_scope(PS_PLANE_0);
        }
        else if (strings_match(buf, "}"))
        {
            commit_brush(&current_brush);
            set_scope(PS_ENTITY);
        }
        break;
    case PS_PLANE_0:
        if (strings_match(buf, ")"))
        {
            component_idx = 0;
            set_scope(PS_PLANE_1);
        }
        else
        {
            switch (component_idx)
            {
            case 0:
                current_face.plane_points.v0.x = atof(buf);
                break;
            case 1:
                current_face.plane_points.v0.y = atof(buf);
                break;
            case 2:
                current_face.plane_points.v0.z = atof(buf);
                break;
            default:
                break;
            }
            component_idx++;
        }
        break;
    case PS_PLANE_1:
        if (strings_match(buf, "("))
        {
            break;
        }
        else if (strings_match(buf, ")"))
        {
            component_idx = 0;
            set_scope(PS_PLANE_2);
        }
        else
        {
            switch (component_idx)
            {
            case 0:
                current_face.plane_points.v1.x = atof(buf);
                break;
            case 1:
                current_face.plane_points.v1.y = atof(buf);
                break;
            case 2:
                current_face.plane_points.v1.z = atof(buf);
                break;
            default:
                break;
            }
            component_idx++;
        }
        break;
    case PS_PLANE_2:
        if (strings_match(buf, "("))
        {
            break;
        }
        else if (strings_match(buf, ")"))
        {
            set_scope(PS_TEXTURE);
        }
        else
        {
            switch (component_idx)
            {
            case 0:
                current_face.plane_points.v2.x = atof(buf);
                break;
            case 1:
                current_face.plane_points.v2.y = atof(buf);
                break;
            case 2:
                current_face.plane_points.v2.z = atof(buf);
                break;
            default:
                break;
            }
            component_idx++;
        }
        break;
    case PS_TEXTURE:
        current_face.texture_idx = map_data_register_texture(buf);
        set_scope(PS_U);
        break;
    case PS_U:
        if (strings_match(buf, "["))
        {
            valve_uvs = true;
            component_idx = 0;
            set_scope(PS_VALVE_U);
        }
        else
        {
            valve_uvs = false;
            current_face.uv_standard.u = atof(buf);
            set_scope(PS_V);
        }
        break;
    case PS_V:
        current_face.uv_standard.v = atof(buf);
        set_scope(PS_ROT);
        break;
    case PS_VALVE_U:
        if (strings_match(buf, "]"))
        {
            component_idx = 0;
            set_scope(PS_VALVE_V);
        }
        else
        {
            switch (component_idx)
            {
            case 0:
                current_face.uv_valve.u.axis.x = atof(buf);
                break;
            case 1:
                current_face.uv_valve.u.axis.y = atof(buf);
                break;
            case 2:
                current_face.uv_valve.u.axis.z = atof(buf);
                break;
            case 3:
                current_face.uv_valve.u.offset = atof(buf);
                break;
            default:
                break;
            }

            component_idx++;
        }
        break;
    case PS_VALVE_V:
        if (strings_match(buf, "["))
        {
            break;
        }
        else if (strings_match(buf, "]"))
        {
            set_scope(PS_ROT);
        }
        else
        {
            switch (component_idx)
            {
            case 0:
                current_face.uv_valve.v.axis.x = atof(buf);
                break;
            case 1:
                current_face.uv_valve.v.axis.y = atof(buf);
                break;
            case 2:
                current_face.uv_valve.v.axis.z = atof(buf);
                break;
            case 3:
                current_face.uv_valve.v.offset = atof(buf);
                break;
            default:
                break;
            }

            component_idx++;
        }
        break;
    case PS_ROT:
        current_face.uv_extra.rot = atof(buf);
        set_scope(PS_U_SCALE);
        break;
    case PS_U_SCALE:
        current_face.uv_extra.scale_x = atof(buf);
        set_scope(PS_V_SCALE);
        break;
    case PS_V_SCALE:
        current_face.uv_extra.scale_y = atof(buf);

        commit_face(&current_face);

        set_scope(PS_BRUSH);
        break;
    default:
        break;
    }
}

void newline()
{
    if (comment)
    {
        comment = false;
    }
}

void commit_face()
{
    vec3 v0v1 = vec3_sub(current_face.plane_points.v1, current_face.plane_points.v0);
    vec3 v1v2 = vec3_sub(current_face.plane_points.v2, current_face.plane_points.v1);
    current_face.plane_normal = vec3_normalize(vec3_cross(v1v2, v0v1));
    current_face.plane_dist = vec3_dot(current_face.plane_normal, current_face.plane_points.v0);
    current_face.is_valve_uv = valve_uvs;

    current_brush.face_count++;
    current_brush.faces = realloc(current_brush.faces, current_brush.face_count * sizeof(face));
    current_brush.faces[current_brush.face_count - 1] = current_face;

    reset_current_face();
}

void commit_brush()
{
    current_entity.brush_count++;
    current_entity.brushes = realloc(current_entity.brushes, current_entity.brush_count * sizeof(brush));

    brush *dest_brush = &current_entity.brushes[current_entity.brush_count - 1];
    *dest_brush = (brush){0};

    dest_brush->face_count = current_brush.face_count;
    dest_brush->faces = realloc(dest_brush->faces, dest_brush->face_count * sizeof(face));
    for (int i = 0; i < dest_brush->face_count; ++i)
    {
        dest_brush->faces[i] = current_brush.faces[i];
    }

    reset_current_brush();
}

void commit_entity()
{
    entity_count++;
    entities = realloc(entities, entity_count * sizeof(entity));

    entity *dest_entity = &entities[entity_count - 1];
    *dest_entity = (entity){0};
    dest_entity->spawn_type = EST_ENTITY;

    dest_entity->property_count = current_entity.property_count;
    dest_entity->properties = realloc(dest_entity->properties, dest_entity->property_count * sizeof(property));
    for (int p = 0; p < dest_entity->property_count; ++p)
    {
        property *dest_property = &dest_entity->properties[p];
        *dest_property = (property){0};

        dest_property->key = STRDUP(current_entity.properties[p].key);
        dest_property->value = STRDUP(current_entity.properties[p].value);
    }

    dest_entity->brush_count = current_entity.brush_count;
    dest_entity->brushes = realloc(dest_entity->brushes, dest_entity->brush_count * sizeof(brush));
    for (int b = 0; b < dest_entity->brush_count; ++b)
    {
        brush *dest_brush = &dest_entity->brushes[b];
        *dest_brush = (brush){0};

        dest_brush->face_count = current_entity.brushes[b].face_count;
        dest_brush->faces = realloc(dest_brush->faces, dest_brush->face_count * sizeof(face));
        for (int f = 0; f < dest_brush->face_count; ++f)
        {
            dest_brush->faces[f] = current_entity.brushes[b].faces[f];
        }
    }

    reset_current_entity();
}

#ifdef __cplusplus
}
#endif
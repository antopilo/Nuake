#include "../h/vector.h"

#include <math.h>

#include "../h/libmap_math.h"
#include "../h/matrix.h"

bool vec3_equals(vec3 lhs, vec3 rhs)
{
    return vec3_length(vec3_sub(lhs, rhs)) < CMP_EPSILON;
}

vec3 vec3_add(vec3 lhs, vec3 rhs)
{
    return (vec3){
        lhs.x + rhs.x,
        lhs.y + rhs.y,
        lhs.z + rhs.z
    };
}

vec3 vec3_sub(vec3 lhs, vec3 rhs)
{
    return (vec3){
        lhs.x - rhs.x,
        lhs.y - rhs.y,
        lhs.z - rhs.z
    };
}

vec3 vec3_mul(vec3 lhs, vec3 rhs)
{
    return (vec3){
        lhs.x * rhs.x,
        lhs.y * rhs.y,
        lhs.z * rhs.z
    };
}

vec3 vec3_div(vec3 lhs, vec3 rhs)
{
    return (vec3){
        lhs.x / rhs.x,
        lhs.y / rhs.y,
        lhs.z / rhs.z
    };
}

double vec3_sqlen(vec3 v)
{
    return vec3_dot(v, v);
}

double vec3_length(vec3 v)
{
    double sqlen = vec3_sqlen(v);
    return (double)sqrt(vec3_sqlen(v));
}

vec3 vec3_normalize(vec3 v)
{
    double len = vec3_length(v);
    return vec3_div_double(v, len);
}

double vec3_dot(vec3 lhs, vec3 rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

vec3 vec3_cross(vec3 lhs, vec3 rhs)
{
    return (vec3){
        lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.x * rhs.y - lhs.y * rhs.x,
    };
}

vec3 vec3_rotate(vec3 v, vec3 axis, double angle)
{
    mat4 mat = rotation_matrix(axis, angle);
    return mat4_mul_vec3(mat, v);
}

vec3 vec3_add_double(const vec3 lhs, const double rhs)
{
    vec3 result = {lhs.x + rhs, lhs.y + rhs, lhs.z + rhs};
    return result;
};

vec3 vec3_sub_double(const vec3 lhs, const double rhs)
{
    vec3 result = {lhs.x - rhs, lhs.y - rhs, lhs.z - rhs};
    return result;
};

vec3 vec3_mul_double(const vec3 lhs, const double rhs)
{
    vec3 result = {lhs.x * rhs, lhs.y * rhs, lhs.z * rhs};
    return result;
};

vec3 vec3_div_double(const vec3 lhs, const double rhs)
{
    vec3 result = {lhs.x / rhs, lhs.y / rhs, lhs.z / rhs};
    return result;
};

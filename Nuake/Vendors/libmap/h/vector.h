#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h>

typedef struct vec3
{
    double x;
    double y;
    double z;
} vec3;

bool vec3_equals(vec3 lhs, vec3 rhs);
vec3 vec3_add(vec3 lhs, vec3 rhs);
vec3 vec3_sub(vec3 lhs, vec3 rhs);
vec3 vec3_mul(vec3 lhs, vec3 rhs);
vec3 vec3_div(vec3 lhs, vec3 rhs);

vec3 vec3_add_double(const vec3 lhs, const double rhs);
vec3 vec3_sub_double(const vec3 lhs, const double rhs);
vec3 vec3_mul_double(const vec3 lhs, const double rhs);
vec3 vec3_div_double(const vec3 lhs, const double rhs);

double vec3_dot(vec3 lhs, vec3 rhs);
vec3 vec3_cross(vec3 lhs, vec3 rhs);
double vec3_sqlen(vec3 v);
double vec3_length(vec3 v);
vec3 vec3_normalize(vec3 v);
vec3 vec3_rotate(vec3 v, vec3 axis, double angle);

#endif

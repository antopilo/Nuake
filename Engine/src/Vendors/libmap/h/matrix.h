#ifndef MATRIX_H
#define MATRIX_H

#include <stdbool.h>

typedef struct vec3 vec3;

typedef struct mat4
{
    double m[16];
} mat4;

mat4 mat4_identity();
//vec4 mat4_mul_vec4(mat4 m, vec4 v);
vec3 mat4_mul_vec3(mat4 m, vec3 v);
double get_fovy(double fov_x, double aspect);
mat4 rotation_matrix(vec3 axis, double angle);
mat4 projection_matrix(double fovy_degrees, double aspect, double z_near, double z_far, bool flip_fov);

#endif

#include "../h/matrix.h"

#include <math.h>

#include "../h/libmap_math.h"
#include "../h/vector.h"

mat4 mat4_identity()
{
    mat4 out = {{1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, 0,
                 0, 0, 0, 1}};
    return out;
}

vec3 mat4_mul_vec3(mat4 m, vec3 v)
{
    vec3 out;
    out.x = m.m[0] * v.x + m.m[4] * v.y + m.m[8] * v.z + m.m[12];
    out.y = m.m[1] * v.x + m.m[5] * v.y + m.m[9] * v.z + m.m[13];
    out.z = m.m[2] * v.x + m.m[6] * v.y + m.m[10] * v.z + m.m[14];
    return out;
}

/*
fvec4 mat4_mul_vec4(mat4 m, vec4 v)
{
    fvec4 out;
    out.x = m.m[0] * v.x + m.m[4] * v.y + m.m[8] * v.z + m.m[12] * v.w;
    out.y = m.m[1] * v.x + m.m[5] * v.y + m.m[9] * v.z + m.m[13] * v.w;
    out.z = m.m[2] * v.x + m.m[6] * v.y + m.m[10] * v.z + m.m[14] * v.w;
    out.w = m.m[3] * v.x + m.m[7] * v.y + m.m[11] * v.z + m.m[15] * v.w;
    return out;
}
*/

mat4 rotation_matrix(vec3 axis, double angle)
{
    angle = DEG_TO_RAD(angle); //converting to radian value
    double u2 = axis.x * axis.x;
    double v2 = axis.y * axis.y;
    double w2 = axis.z * axis.z;
    double L = u2 + v2 + w2;

    return (mat4){
        {(u2 + (v2 + w2) * cos(angle)) / L, (axis.x * axis.y * (1.0 - cos(angle)) - axis.z * sqrt(L) * sin(angle)) / L, (axis.x * axis.z * (1 - cos(angle)) + axis.y * sqrt(L) * sin(angle)) / L, 0,
         (axis.x * axis.y * (1 - cos(angle)) + axis.z * sqrt(L) * sin(angle)) / L, (v2 + (u2 + w2) * cos(angle)) / L, (axis.y * axis.z * (1 - cos(angle)) - axis.x * sqrt(L) * sin(angle)) / L, 0,
         (axis.x * axis.z * (1 - cos(angle)) - axis.y * sqrt(L) * sin(angle)) / L, (axis.y * axis.z * (1 - cos(angle)) + axis.x * sqrt(L) * sin(angle)) / L, (w2 + (u2 + v2) * cos(angle)) / L, 0,
         0, 0, 0, 1}};
}

double get_fovy(double fov_x, double aspect)
{
    return RAD_TO_DEG(atan(aspect * tan(DEG_TO_RAD(fov_x * 0.5)))) * 2.0;
}

mat4 projection_matrix(double fovy_degrees, double aspect, double z_near, double z_far, bool flip_fov)
{
    if (flip_fov)
    {
        fovy_degrees = get_fovy(fovy_degrees, 1.0f / aspect);
    }

    double delta_z = z_far - z_near;
    double radians = (double)(fovy_degrees / 2.0 * PI / 180.0);
    double sine = (double)sin(radians);

    if (delta_z == 0 || sine == 0 || aspect == 0)
    {
        return mat4_identity();
    }

    double cotangent = (double)(cos(radians) / sine);

    return (mat4){
        {cotangent / aspect, 0, 0, 0,
         0, cotangent, 0, 0,
         0, 0, -(z_far + z_near) / delta_z, 1,
         0, 0, -2 * z_near * z_far / delta_z, 0}};
}

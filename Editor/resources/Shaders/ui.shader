#shader vertex
#version 460 core
layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 UV;
uniform mat4 model;
uniform mat4 projection;


out vec2 a_UV;

void main()
{
    a_UV = UV;
    gl_Position = projection * model * vec4(Position, 1.0);
}

#shader fragment
#version 460 core

uniform vec4 background_color;
uniform float u_border_radius;
uniform vec2 u_size;

out vec4 FragColor;

in vec2 a_UV;

bool ShouldDiscard(vec2 coords, vec2 dimensions, float radius)
{
    vec2 circle_center = vec2(radius, radius);

    if (length(coords - circle_center) > radius
        && coords.x < circle_center.x && coords.y < circle_center.y) return true; //first circle

    circle_center.x += dimensions.x - 2 * radius;

    if (length(coords - circle_center) > radius
        && coords.x > circle_center.x && coords.y < circle_center.y) return true; //second circle

    circle_center.y += dimensions.y - 2 * radius;

    if (length(coords - circle_center) > radius
        && coords.x > circle_center.x && coords.y > circle_center.y) return true; //third circle

    circle_center.x -= dimensions.x - 2 * radius;

    if (length(coords - circle_center) > radius
        && coords.x < circle_center.x && coords.y > circle_center.y) return true; //fourth circle

    return false;

}

void main()
{
    vec2 coords = a_UV * u_size;
    vec2 center = u_size / 2.0;

    // Border rounding
    if (ShouldDiscard(coords, u_size, u_border_radius))
        discard;
    FragColor = background_color;
}

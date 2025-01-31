#shader vertex
#version 440 core
layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 UV;

uniform mat4 model;
uniform mat4 projection;

out sample vec2 a_UV;

void main()
{
    a_UV = UV;
    gl_Position = projection * model * vec4(Position, 1.0);
}

#shader fragment
#version 440 core

uniform vec4 u_BackgroundColor;

uniform int u_HasBackgroundTexture;
uniform sampler2D u_BackgroundTexture;

uniform float u_BorderRadius;
uniform vec2 u_Size;

out vec4 FragColor;

in vec2 a_UV;

bool ShouldDiscard(vec2 coords, vec2 dimensions, float radius)
{
    vec2 circle_center = vec2(radius, radius);

    if (length(coords - circle_center) >= radius
        && coords.x < circle_center.x && coords.y < circle_center.y) return true; //first circle

    circle_center.x += dimensions.x - 2 * radius;

    if (length(coords - circle_center) >= radius
        && coords.x > circle_center.x && coords.y < circle_center.y) return true; //second circle

    circle_center.y += dimensions.y - 2 * radius;

    if (length(coords - circle_center) >= radius
        && coords.x > circle_center.x && coords.y > circle_center.y) return true; //third circle

    circle_center.x -= dimensions.x - 2 * radius;

    if (length(coords - circle_center) >= radius
        && coords.x < circle_center.x && coords.y > circle_center.y) return true; //fourth circle

    return false;

}

void main()
{
    vec2 coords = a_UV * u_Size;
    vec2 center = u_Size / 2.0;

    // Border rounding
    if (ShouldDiscard(coords, u_Size, u_BorderRadius))
        discard;

    FragColor = u_BackgroundColor;

    if (u_HasBackgroundTexture == 1)
    {
        vec2 uv = a_UV;
        uv.y = 1.0 - a_UV.y;
        FragColor = texture(u_BackgroundTexture, uv);
    }
}

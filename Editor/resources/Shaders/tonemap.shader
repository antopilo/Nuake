#shader vertex
#version 460 core

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 UVPosition;

out flat vec2 UV;

void main()
{
    UV = UVPosition;
    gl_Position = vec4(VertexPosition, 1.0f);
}

#shader fragment
#version 460 core

uniform sampler2D u_Source;

in vec2 UV;

out vec4 FragColor;

void main()
{
    vec4 color = texture(u_Source, UV);
    color = color / (color + vec3(1.0));
    const float gamma = 2.2;
    // HDR tonemapping
    color = vec3(1.0) - exp(-color * u_Exposure);
    // gamma correct
    color = pow(color, vec3(1.0 / gamma));
    FragColor = color;
}
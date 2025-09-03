#shader vertex
#version 440 core

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 UVPosition;

out flat vec2 UV;

void main()
{
    UV = UVPosition;
    gl_Position = vec4(VertexPosition, 1.0f);
}

#shader fragment
#version 440 core

uniform sampler2D u_Source;

in vec2 UV;

out vec4 FragColor;

void main()
{
    FragColor = texture(u_Source, UV);
}
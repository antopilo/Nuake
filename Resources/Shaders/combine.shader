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
uniform sampler2D u_Source2;

in vec2 UV;

out vec4 FragColor;

void main()
{
    vec4 a = texture(u_Source, UV);
    vec4 b = texture(u_Source2, UV);

    FragColor = vec4(vec3(a.rgb + (b.rgb) / 2.0), a.a);
}
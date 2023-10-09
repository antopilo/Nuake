#shader vertex
#version 440 core

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec4 VertexColor;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;

out vec4 LineColor;

void main()
{
    LineColor = VertexColor;
    gl_Position = u_Projection * u_View * vec4(VertexPosition, 1.0f);
}

#shader fragment
#version 440 core

in vec4 LineColor;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out int gEntityID;

uniform float u_Opacity;
uniform int u_EntityID;

void main()
{
    FragColor = LineColor * vec4(1, 1, 1, u_Opacity);
    gEntityID = int(u_EntityID);
}
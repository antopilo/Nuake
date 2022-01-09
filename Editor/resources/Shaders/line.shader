#shader vertex
#version 460 core

layout(location = 0) in vec3 VertexPosition;
layout(location = 0) in vec4 VertexColor;

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
#version 460 core

in vec4 LineColor;

out vec4 FragColor;

void main()
{
    FragColor = LineColor;
}
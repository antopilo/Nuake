#shader vertex
#version 460 core


layout(location = 0) in vec3 VertexPosition;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;
void main()
{
    gl_Position = u_Projection * u_View * u_Model * vec4(VertexPosition, 1.0f);
}

#shader fragment
#version 460 core

uniform vec4 u_Color;
out vec4 FragColor;

void main()
{
    FragColor = u_Color;
}
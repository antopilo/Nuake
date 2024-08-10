#shader vertex
#version 440 core

layout(location = 0) in vec3 VertexPosition;

uniform vec3 u_StartPos;
uniform vec3 u_EndPos;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;

void main()
{
    vec3 pos = mix(u_StartPos, u_EndPos, VertexPosition.x);
    gl_Position = u_Projection * u_View * vec4(pos, 1.0f);
}

#shader fragment
#version 440 core

layout(location = 0) out vec4 FragColor;

uniform vec4 u_Color;

void main()
{
    FragColor = u_Color;
}
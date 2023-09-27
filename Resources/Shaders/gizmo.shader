#shader vertex
#version 460 core
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 1) in vec2 UV;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out sample vec2 a_UV;

void main()
{
    a_UV = UV;
    gl_Position = u_Projection * u_View * u_Model * vec4(Position, 1.0);
}

#shader fragment
#version 460 core

out vec4 FragColor;

in vec2 a_UV;

uniform sampler2D gizmo_texture;

void main() 
{
    vec4 px_color = texture(gizmo_texture, a_UV).rgba;
    FragColor = px_color * vec4(1, 1, 1, 0.5);

}
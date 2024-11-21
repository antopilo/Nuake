#shader vertex
#version 440 core
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 1) in vec2 UV;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out mat4 o_Projection;
out sample vec2 a_UV;
out vec4 o_FragPos;

void main()
{
    a_UV = UV;
    o_Projection = u_Projection;
    gl_Position = u_Projection * u_View * u_Model * vec4(Position, 1.0);
    o_FragPos = gl_Position;
}

#shader fragment
#version 440 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out int gEntityID;

in vec2 a_UV;
in mat4 o_Projection;
in vec4 o_FragPos;

uniform sampler2D u_DepthTexture;
uniform sampler2D gizmo_texture;
uniform float u_Opacity;
uniform int u_EntityID;

void main() 
{
    vec4 color = vec4(0, 0, 0, 0);

    vec4 px_color = texture(gizmo_texture, a_UV).rgba;
    color = px_color * vec4(1, 1, 1, u_Opacity);

    if(color.a == 0.0f)
    {
        discard;
    }

    if(u_Opacity >= 0.5f)
    {
        gEntityID = u_EntityID;
    }

    FragColor = color;
}
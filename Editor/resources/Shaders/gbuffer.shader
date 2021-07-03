#shader vertex
#version 460 core

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 UVPosition;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 Tangent;
layout(location = 4) in vec3 Bitangent;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 FragPos;
out mat3 TBN;
out vec2 UV;

void main()
{
    vec3 N = normalize((u_Model * vec4(Normal, 0.0f)).xyz);
    vec3 T = normalize((u_Model * vec4(Tangent, 0.0f)).xyz);
    vec3 B = normalize((u_Model * vec4(Bitangent, 0.0f)).xyz);
    TBN = mat3(T, B, N);

    UV = UVPosition;

    gl_Position = u_Projection * u_View * u_Model * vec4(VertexPosition, 1.0f);
}

#shader fragment
#version 460 core

layout(location = 0) out vec3 gAlbedo;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gMaterial;

in vec3 FragPos;
in vec2 UV;
in mat3 TBN;

uniform sampler2D m_Albedo;
uniform sampler2D m_Normal;
uniform sampler2D m_Roughness;
uniform sampler2D m_Metalness;
uniform sampler2D m_AO;

void main()
{
    gAlbedo.rgb = texture(m_Albedo, UV).rgb;

    vec3 normal = texture(m_Normal, UV).rgb;
    normal = normal * 2.0 - 1.0;
    normal = normalize((TBN * normalize(normal)).xyz);
    
    gNormal = (normal * 0.5 + 0.5);
    gMaterial.r = texture(m_Metalness, UV).r;
    gMaterial.g = texture(m_AO, UV).r;
    gMaterial.b = texture(m_Roughness, UV).r;
}
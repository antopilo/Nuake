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

float u_NearPlane = 0.1f;
float u_FarPlane = 25.5f;
in vec2 UV;

out float3 FragColor;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * u_NearPlane * u_FarPlane) / (u_FarPlane + u_NearPlane - z * (u_FarPlane - u_NearPlane));
}

void main()
{
    float depthValue = texture(u_Source, UV).r;
    float linearDepth = LinearizeDepth(depthValue) / u_FarPlane; // Normalize to [0, 1] range
    vec3 color = vec3(linearDepth);
    gl_FragColor = vec4(color, 1.0);
}
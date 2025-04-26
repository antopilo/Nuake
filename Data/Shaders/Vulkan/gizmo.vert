#include "Utils/header.hlsl"

struct DebugConstant
{
    float4 Color;
    float4x4 Transform;
    int TextureID;
    float EntityID;
};

[[vk::push_constant]]
DebugConstant pushConstants;

// Outputs
struct VSOutput 
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
};

// Main vertex shader
VSOutput main(uint vertexIndex : SV_VertexID) 
{
    VSOutput output;

    Vertex v = vertexBuffer[vertexIndex];
    output.UV = float2(v.uv_x, v.uv_y);
    output.Position = mul(pushConstants.Transform, float4(v.position, 1.0f));
    
    return output;
}
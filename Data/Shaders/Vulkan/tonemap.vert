#include "Utils/header.hlsl"

struct TonemapPushConstant
{
    float Exposure;
    float Gamma;
    int SourceTextureID;
};

[[vk::push_constant]]
TonemapPushConstant pushConstants;

// Outputs
struct VSOutput {
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
};


// Main vertex shader
VSOutput main(uint vertexIndex : SV_VertexID) 
{
    VSOutput output;

    Vertex v = vertexBuffer[vertexIndex];
    output.UV = float2(v.uv_x, v.uv_y);
    output.Position = float4(v.position, 1.0f);
    
    return output;
}
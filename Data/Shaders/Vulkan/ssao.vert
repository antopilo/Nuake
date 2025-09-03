#include "Utils/header.hlsl"

[[vk::binding(0, 7)]]
StructuredBuffer<float3> ssaoKernels;

struct SSAOConstant
{
    int noiseTextureID;
    int normalTextureID;
    int depthTextureID;
    float radius;
    float bias;
    float2 noiseScale;
    float power;
};

[[vk::push_constant]]
SSAOConstant pushConstants;

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
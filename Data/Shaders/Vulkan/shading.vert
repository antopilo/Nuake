#include "Utils/header.hlsl"

struct ShadingPushConstant
{
    int AlbedoInputTextureId;
    int DepthInputTextureId;
    int NormalInputTextureId;
    int MaterialInputTextureId;
    int LightOffset;
    int LightCount;
    int CameraID;
    float AmbientTerm;
    float cascadeDepth[4];
    int SSAOTextureId;
    int EntityTextureId;
};

[[vk::push_constant]]
ShadingPushConstant pushConstants;

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
    output.Position = float4(v.position, 1.0f);
    
    return output;
}
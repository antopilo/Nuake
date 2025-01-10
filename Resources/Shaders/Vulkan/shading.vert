struct Camera
{
    float4x4 view;
    float4x4 proj;
    float4x4 invView;
    float4x4 invProj;
};
[[vk::binding(0, 0)]]
StructuredBuffer<Camera> camera : register(t0);

struct ModelData
{
    float4x4 model;
};
[[vk::binding(0, 1)]]
StructuredBuffer<ModelData> model : register(t1);

struct Vertex 
{
    float3 position;
    float uv_x;
    float3 normal;
    float uv_y;
    float3 tangent;
    float3 bitangent;
};

[[vk::binding(0, 2)]] 
StructuredBuffer<Vertex> vertexBuffer : register(t2);

struct ShadingPushConstant
{
    int AlbedoInputTextureId;
    int DepthInputTextureId;
    int NormalInputTextureId;
    int MaterialInputTextureId;
};

[[vk::push_constant]]
ShadingPushConstant pushConstants;

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
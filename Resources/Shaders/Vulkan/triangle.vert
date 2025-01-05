struct Camera
{
    float4x4 view;
    float4x4 proj;
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

struct ModelPushConstant
{
    int modelIndex;  // Push constant data
    int materialIndex;
};

[[vk::push_constant]]
ModelPushConstant pushConstants;

// Outputs
struct VSOutput {
    float4 Position : SV_Position;
    float3 Color : TEXCOORD0;
    float2 UV : TEXCOORD1;
    float3 Normal : TEXCOORD2;
    float3x3 TBN : TEXCOORD3;
};

// Main vertex shader
VSOutput main(uint vertexIndex : SV_VertexID) 
{
    VSOutput output;

    Camera camData = camera[0];

    ModelData modelData = model[pushConstants.modelIndex];

    // Load vertex data from the buffer
    Vertex v = vertexBuffer[vertexIndex];

    // Output the position of each vertex
    output.Position = mul(camData.proj, mul(camData.view, mul(modelData.model, float4(v.position, 1.0f))));
    output.Color = normalize(float3(v.position.xyz));
    output.UV = float2(v.uv_x, v.uv_y);
    output.Normal = normalize(v.normal);

    float3 T = normalize(mul((float3x3)modelData.model, normalize(v.tangent.xyz)));
    float3 B = normalize(mul((float3x3)modelData.model, normalize(v.bitangent.xyz)));
    float3 N = normalize(mul((float3x3)modelData.model, normalize(v.normal)).xyz);
    output.TBN = transpose(float3x3(T, B, N));
    return output;
}
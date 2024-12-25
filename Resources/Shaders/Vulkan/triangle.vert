struct World
{
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};

[[vk::binding(0, 0)]]
StructuredBuffer<World> world : register(t0);

struct Vertex 
{
    float3 position;
    float uv_x;
    float3 normal;
    float uv_y;
    float4 tangent;
    float4 bitangent;
};

[[vk::binding(0, 1)]] 
StructuredBuffer<Vertex> vertexBuffer : register(t1); 

cbuffer PushConstants : register(b0) 
{ 
    float4x4 render_matrix;
};

// Outputs
struct VSOutput {
    float4 Position : SV_Position;
    float3 Color : TEXCOORD0;
    float2 UV : TEXCOORD1;
};

// Main vertex shader
VSOutput main(uint vertexIndex : SV_VertexID) 
{
    VSOutput output;

    World worldData = world[0];
    
    // Load vertex data from the buffer
    Vertex v = vertexBuffer[vertexIndex];

    // Output the position of each vertex
    output.Position = mul(worldData.proj, mul(worldData.view, mul(worldData.model, float4(v.position, 1.0f))));
    output.Color = float3(v.normal.xyz);

    return output;
}
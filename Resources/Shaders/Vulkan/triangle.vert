// struct World
// {
//     mat4x4 view;
//     mat4x4 proj;
// }
// World worldSettings : register(t0);

struct Vertex 
{
    float3 position;
    float uv_x;
    float3 normal;
    float uv_y;
    float4 color;
};
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

    // Load vertex data from the buffer
    Vertex v = vertexBuffer[vertexIndex];

    // Output the position of each vertex
    output.Position = float4(v.position, 1.0f);
    output.Color = v.color;

    return output;
}
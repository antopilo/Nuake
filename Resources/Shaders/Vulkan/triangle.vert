struct Vertex 
{
    float3 position;
    float uv_x;
    float3 normal;
    float uv_y;
    float4 color;
};

// Define the structured buffer for vertices
StructuredBuffer<Vertex> vertexBuffer : register(t0); // Binding of vertex buffer (example: t0)

// Define push constants block
cbuffer PushConstants : register(b0) { // Push constants binding (example: b0)
    float4x4 render_matrix; // Matrix for rendering
    uint64_t vertexBufferAddress; // Buffer reference address (Vulkan-specific handling required)
};

// Outputs
struct VSOutput {
    float4 Position : SV_Position;
    float3 Color : TEXCOORD0;
    float2 UV : TEXCOORD1;
};

// Main vertex shader
VSOutput main(uint vertexIndex : SV_VertexID) {
    VSOutput output;

    // Load vertex data from the buffer
    Vertex v = vertexBuffer[vertexIndex];

    // Transform and output vertex data
    output.Position = mul(render_matrix, float4(v.position, 1.0f));
    output.Color = v.color.xyz;
    output.UV = float2(v.uv_x, v.uv_y);

    return output;
}
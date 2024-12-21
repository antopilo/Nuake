struct Vertex 
{
    float3 position;
    float uv_x;
    float3 normal;
    float uv_y;
    float4 color;
};

// Define the structured buffer for vertices
StructuredBuffer<Vertex> vertexBuffer : register(t1); // Binding of vertex buffer (example: t0)

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

        // Constant array of positions for the triangle
    float3 positions[3] = {
        float3(1.0f,  1.0f, 0.0f),
        float3(-1.0f, 1.0f, 0.0f),
        float3(0.0f, -1.0f, 0.0f)
    };

    // Constant array of colors for the triangle
    float3 colors[3] = {
        float3(1.0f, 0.0f, 0.0f), // red
        float3(0.0f, 1.0f, 0.0f), // green
        float3(0.0f, 0.0f, 1.0f)  // blue
    };
    // Load vertex data from the buffer
    Vertex v = vertexBuffer[vertexIndex];

    // Output the position of each vertex
    output.Position = float4(v.position, 1.0f);
    output.Color = v.color;

    return output;
}
// HLSL version for Shader Model 6.1
struct VSOutput {
    float4 Position : SV_Position;
    float3 Color : TEXCOORD0;
};

VSOutput main(uint vertexIndex : SV_VertexID) 
{
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

    // Output the position of each vertex
    output.Position = float4(positions[vertexIndex], 1.0f);
    output.Color = colors[vertexIndex];

    return output;
}
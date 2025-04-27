#include "Utils/header.hlsl"

struct ModelPushConstant
{
    int modelIndex;  // Push constant data
    int materialIndex;
    int cameraID;
};

[[vk::push_constant]]
ModelPushConstant pushConstants;

// Outputs
struct VSOutput 
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
};

float LinearizeDepth(float depth, float nearPlane, float farPlane, bool reverseDepth)
{
    if (reverseDepth)
    {
        // Reverse depth (near plane = 1.0, far plane = 0.0)
        return nearPlane * farPlane / lerp(farPlane, nearPlane, depth);
    }
    else
    {
        // Standard depth (near plane = 0.0, far plane = 1.0)
        return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - depth * (farPlane - nearPlane));
    }
}

// Main vertex shader
VSOutput main(uint vertexIndex : SV_VertexID) 
{
    VSOutput output;

    ModelData modelData = model[pushConstants.modelIndex];
    CameraView  camView = cameras[pushConstants.cameraID];

    // Load vertex data from the buffer
    Vertex v = vertexBuffer[vertexIndex];

    // Output the position of each vertex
    output.Position = mul(camView.Projection, mul(camView.View,mul(modelData.model, float4(v.position, 1.0f))));
    output.UV = float2(v.uv_x, v.uv_y);

    //output.Position.z = LinearizeDepth(output.Position.z, camView.Near, camView.Far, false);
    return output;
}
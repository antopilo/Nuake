#include "Utils/header.hlsl"

struct ModelPushConstant
{
    int modelIndex;  // Push constant data
    int materialIndex;
    int cameraID;
    float entityID;
};

[[vk::push_constant]]
ModelPushConstant pushConstants;

// Outputs
struct VSOutput 
{
    float4 Position : SV_Position;
    float3 Color : TEXCOORD0;
    float2 UV : TEXCOORD1;
    float3 Normal : TEXCOORD2;
    float3 Tangent : TEXCOORD3;
    float3 Bitangent : TEXCOORD4;
};

float3x3 invert(float3x3 m)
{
    float3 a = m[0];
    float3 b = m[1];
    float3 c = m[2];

    float3 r0 = cross(b, c);
    float3 r1 = cross(c, a);
    float3 r2 = cross(a, b);

    float det = dot(r2, c);

    // Return identity if not invertible (optional fallback)
    if (abs(det) < 1e-6)
        return float3x3(1.0, 0.0, 0.0,
                        0.0, 1.0, 0.0,
                        0.0, 0.0, 1.0);

    float invDet = 1.0 / det;

    return float3x3(r0 * invDet,
                    r1 * invDet,
                    r2 * invDet);
}

// Main vertex shader
VSOutput main(uint vertexIndex : SV_VertexID) 
{
    VSOutput output;

    CameraView camView = cameras[pushConstants.cameraID];
    ModelData modelData = model[pushConstants.modelIndex];

    // Load vertex data from the buffer
    Vertex v = vertexBuffer[vertexIndex];

    // Output the position of each vertex
    output.Position = mul(camView.Projection, mul(camView.View, mul(modelData.model, float4(v.position, 1.0f))));
    output.Color = normalize(float3(v.position.xyz));
    output.UV = float2(v.uv_x, v.uv_y);
    
    float3x3 upper3x3 = (float3x3)modelData.model;
    float3x3 normalMatrix = transpose(invert(upper3x3));
    output.Bitangent = mul(normalMatrix, normalize(v.bitangent.xyz));
    output.Normal = mul(normalMatrix, normalize(v.normal.xyz));
    output.Tangent = mul(normalMatrix, normalize(v.tangent.xyz));

    //output.Normal = v.normal.xyz;
    return output;
}
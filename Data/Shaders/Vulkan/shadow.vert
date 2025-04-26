// Transforms
struct ModelData
{
    float4x4 model;
};
[[vk::binding(0, 0)]]
StructuredBuffer<ModelData> model : register(t1);

// Vertex
struct Vertex 
{
    float3 position;
    float uv_x;
    float3 normal;
    float uv_y;
    float3 tangent;
    float3 bitangent;
};

[[vk::binding(0, 1)]] 
StructuredBuffer<Vertex> vertexBuffer : register(t2);

// Samplers
[[vk::binding(0, 2)]]
SamplerState mySampler[2] : register(s0);

// Materials
struct Material
{
    float hasAlbedo;
    float3 albedo;
    int hasNormal;
    int hasMetalness;
    int hasRoughness;
    int hasAO;
    float metalnessValue;
    float roughnessValue;
    float aoValue;
    int albedoTextureId;
    int normalTextureId;
    int metalnessTextureId;
    int roughnessTextureId;
    int aoTextureId;
};
[[vk::binding(0, 3)]]
StructuredBuffer<Material> material;

// Textures
[[vk::binding(0, 4)]]
Texture2D textures[];

// Lights
struct Light
{
    float3 position;
    int type;
    float4 color;
    float3 direction;
    float outerConeAngle;
    float innerConeAngle;
    bool castShadow;
    int shadowMapTextureId[4];
    int transformId[4];
};

[[vk::binding(0, 5)]]
StructuredBuffer<Light> lights;

// Cameras
struct CameraView {
    float4x4 View;
    float4x4 Projection;
    float4x4 ViewProjection;
    float4x4 InverseView;
    float4x4 InverseProjection;
    float3 Position;
    float Near;
    float Far;
};
[[vk::binding(0, 6)]]
StructuredBuffer<CameraView> cameras;

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

    //output.Position.z = LinearizeDepth(output.Position.z, camView.Near, camView.Far, false);
    return output;
}
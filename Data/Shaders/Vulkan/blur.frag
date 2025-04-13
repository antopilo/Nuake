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
SamplerState mySampler : register(s0);

// Materials
struct Material
{
    bool hasAlbedo;
    float3 albedo;
    bool hasNormal;
    bool hasMetalness;
    bool hasRoughness;
    bool hasAO;
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

[[vk::binding(0, 7)]]
StructuredBuffer<float3> ssaoKernels;

struct PSInput {
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
};

struct PSOutput {
    float4 oColor0 : SV_TARGET;
};

struct BlurConstant
{
    int blurSourceID;
    float2 sourceSize;
};

[[vk::push_constant]]
BlurConstant pushConstants;

float3 SampleTexture(int textureId, float2 uv)
{
    return textures[textureId].Sample(mySampler, uv).rgb;
}

PSOutput main(PSInput input)
{
    float2 texelSize = 1.0 / pushConstants.sourceSize;
    float3 result = 0.0;
    for (int x = -2; x < 2; x++)
    {
        for (int y = -2; y < 2; y++)
        {
            float2 offset = float2(x, y) * texelSize;
            result += SampleTexture(pushConstants.blurSourceID, input.UV + offset);
        }
    }

    result = result / (4.0 * 4.0);

    PSOutput output;
    output.oColor0 = float4(result.rgb, 1.0f);
    return output;
}
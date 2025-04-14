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

struct PSInput {
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
};

struct PSOutput {
    float4 oColor0 : SV_TARGET;
    float4 oEntityID : SV_TARGET1;
};

struct DebugConstant
{
    float4 Color;
    float4x4 Transform;
    int TextureID;
    float EntityID;
};

[[vk::push_constant]]
DebugConstant pushConstants;

PSOutput main(PSInput input)
{
    PSOutput output;

    if(pushConstants.TextureID < 0)
    {
        output.oColor0 = float4(input.UV.x, input.UV.y, 0, 1);
    }
    else
    {
        float2 uv = input.UV;
        float4 textureSample = textures[pushConstants.TextureID].Sample(mySampler, uv);

        // Alpha scisorring
        if(textureSample.a < 0.1)
        {
            //discard;
        }

        output.oColor0 = textureSample * pushConstants.Color;

        if(pushConstants.EntityID != 0.0f)
        {
            float2 center = float2(0.5, 0.5);
            float dist = distance(uv, center);
            float radius = 0.5; // You can adjust this as needed

            if (dist <= radius)
            {
                output.oEntityID = float4(pushConstants.EntityID, 0, 0, 1.0f);
            }
            else
            {
                output.oEntityID = float4(0, 0, 0, 0); // Or leave it unassigned if default is zero
                discard;
            }
        }
    }

    return output;
}
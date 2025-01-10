struct Camera
{
    float4x4 view;
    float4x4 proj;
    float4x4 invView;
    float4x4 invProj;
    float3 position;
};
[[vk::binding(0, 0)]]
StructuredBuffer<Camera> camera : register(t0);

[[vk::binding(0, 3)]]
SamplerState mySampler : register(s0);       // Sampler binding at slot s0

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

[[vk::binding(0, 4)]]
StructuredBuffer<Material> material; // array de 2000 materials

[[vk::binding(0, 5)]]
Texture2D textures[]; // Array de 500 textures

struct Light
{
    float3 position;
    int type;
    float4 color;
    float3 direction;
    float outerConeAngle;
    float innerConeAngle;
    bool castShadow;
    int shadowMapTextureId;
    int transformId;
};

[[vk::binding(0, 6)]]
StructuredBuffer<Light> lights;

struct PSInput {
    float4 Position : SV_Position;
};

struct PSOutput {
    float4 oColor0 : SV_TARGET;
};

struct ModelPushConstant
{
    int modelIndex;  // Push constant data
    int materialIndex;
};

[[vk::push_constant]]
ModelPushConstant pushConstants;

PSOutput main(PSInput input)
{
    PSOutput output;
    return output;
}
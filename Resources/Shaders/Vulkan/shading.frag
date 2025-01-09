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
};
[[vk::binding(0, 4)]]
StructuredBuffer<Material> material;

[[vk::binding(0, 5)]]
Texture2D textures[]; // Array de 500 textures

struct PSInput {
    float2 UV : TEXCOORD0;
    float4x4 InvProj : TEXCOORD1;
    float4x4 InvView : TEXCOORD2;
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

    output.oColor0 = float4(1, 0, 0, 1);

    return output;
}
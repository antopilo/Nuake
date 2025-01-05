[[vk::binding(0, 3)]]
Texture2D<float3> albedo : register(t1); // Texture binding at slot t0

[[vk::binding(0, 4)]]
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
[[vk::binding(0, 5)]]
StructuredBuffer<Material> material;

struct PSInput {
    float4 Position : SV_Position;
    float3 Color : TEXCOORD0;
    float2 UV : TEXCOORD1;
    float3 Normal : TEXCOORD2;
};

struct PSOutput {
    float4 oColor0 : SV_TARGET;
    float4 oNormal : SV_TARGET1;
    float4 oMaterial : SV_TARGET2;
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
    Material inMaterial = material[pushConstants.materialIndex];
    float3 textureColor = albedo.Sample(mySampler, input.UV);
    output.oColor0 = float4(textureColor, 1.0f);
    output.oNormal = float4(input.Normal, 1.0f);
    //output.oMaterial = float4(inMaterial.metalnessValue, inMaterial.aoValue, inMaterial.roughnessValue, 1.0f);
    output.oMaterial = float4(inMaterial.albedo.xyz, 1.0f);
    return output;
}
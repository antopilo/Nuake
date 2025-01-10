struct Camera
{
    float4x4 view;
    float4x4 proj;
    float4x4 invView;
    float4x4 invProj;
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

struct PSInput {
    float4 Position : SV_Position;
    float3 Color : TEXCOORD0;
    float2 UV : TEXCOORD1;
    float3 Normal : TEXCOORD2;
    float3x3 TBN : TEXCOORD3;
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
    // NORMAL
    // TODO use TBN matrix
    float3 normal = float3(0.0, 0.0f, 1.0f);
    if(inMaterial.hasNormal == 1)
    {
        // Sample from texture.
    }

    normal = mul(input.TBN, normal);
    normal = normal / 2.0f + 0.5f;
    output.oNormal = float4(float3(1, 0, 0), 1.0f);

    // MATERIAL

    // ALBEDO COLOR
    float4 albedoColor = float4(inMaterial.albedo.xyz, 1.0f);
    if(inMaterial.hasAlbedo == 1)
    {
        float4 albedoSample = textures[inMaterial.albedoTextureId].Sample(mySampler, input.UV);

        // Alpha cutout?
        if(albedoSample.a < 0.001f)
        {
            discard;
        }

        albedoColor.xyz = albedoSample.xyz;
    }
    output.oColor0 = albedoColor;
    output.oColor0 = float4(normal, 1.0);
    // MATERIAL PROPERTIES
    float metalnessValue = inMaterial.metalnessValue;
    if(inMaterial.hasMetalness == 1)
    {
        // TODO: Sample from metal texture
    }
    
    float aoValue = inMaterial.aoValue;
    if(inMaterial.hasAO == 1)
    {
        // TODO: Sample from AO texture
    }

    float roughnessValue = inMaterial.roughnessValue;
    if(inMaterial.hasRoughness == 1)
    {
        // TODO: Sample from roughness texture
    }

    float3 materialOuput = float3(inMaterial.metalnessValue, inMaterial.aoValue, inMaterial.roughnessValue);
    
    output.oMaterial = float4(materialOuput, 1.0f);
    return output;
}
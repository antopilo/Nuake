#include "Utils/header.hlsl"

struct PSInput 
{
    float4 Position : SV_Position;
    float3 Color : TEXCOORD0;
    float2 UV : TEXCOORD1;
    float3 Normal : TEXCOORD2;
    float3 Tangent : TEXCOORD3;
    float3 Bitangent : TEXCOORD4;
};

struct PSOutput {
    float4 oColor0 : SV_TARGET;
    float4 oNormal : SV_TARGET1;
    float4 oMaterial : SV_TARGET2;
    float4 oEntityID : SV_TARGET3;
};

struct ModelPushConstant
{
    int modelIndex;  // Push constant data
    int materialIndex;
    int cameraID;
    float entityID;
};

[[vk::push_constant]]
ModelPushConstant pushConstants;



PSOutput main(PSInput input)
{
    PSOutput output;

    Material inMaterial = material[pushConstants.materialIndex]; 

    SamplerState samplerr = mySampler[inMaterial.samplingType];
    // NORMAL
    // TODO use TBN matrix
    float3 T = input.Tangent.xyz;
    float3 B = input.Bitangent.xyz;
    float3 N = input.Normal.xyz;
    float3x3 TBN = float3x3(T, B, N);

    float3 normal = float3(0.0, 0.0, 1.0);
    if(inMaterial.hasNormal == 1)
    {
        normal = textures[inMaterial.normalTextureId].Sample(samplerr, input.UV).rgb;
        normal.xyz = normal.zxy;
        normal = normal * 2.0f - 1.0f;
    }
    else
    {
        normal = normal;
    }

    //normal = input.Normal;
    normal = mul(transpose(TBN), normal);
    normal = normal / 2.0f + 0.5f;
    output.oNormal = float4(normal, 1.0f);

    // MATERIAL

    // ALBEDO COLOR
    float4 albedoColor = float4(inMaterial.albedo.xyz, 1.0f);
    if(inMaterial.hasAlbedo == 1)
    {
        float4 albedoSample = textures[inMaterial.albedoTextureId].Sample(samplerr, input.UV);

        // Alpha cutout?
        if(albedoSample.a < 0.001f)
        {
            discard;
        }

        albedoColor.xyz = albedoSample.xyz;
    }
    output.oColor0 = albedoColor;

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

    output.oEntityID = float4(pushConstants.entityID, pushConstants.materialIndex, 0.0f, 1.0f);

    return output;
}
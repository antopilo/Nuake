#include "Utils/header.hlsl"

struct PSInput 
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
};

struct ModelPushConstant
{
    int modelIndex;  // Push constant data
    int materialIndex;
    int cameraID;
};

[[vk::push_constant]]
ModelPushConstant pushConstants;

void main(PSInput input)
{
    Material inMaterial = material[pushConstants.materialIndex];
    if(inMaterial.alphaScissor == 1)
    {
        SamplerState samplerr = mySampler[inMaterial.samplingType];
        float albedoAlpha = textures[inMaterial.albedoTextureId].Sample(samplerr, input.UV).a;
        if(albedoAlpha < 0.1f)
        {
            discard;
        }
    }
}
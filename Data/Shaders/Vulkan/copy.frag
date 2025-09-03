#include "Utils/header.hlsl"

struct PSInput 
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
};

struct PSOutput {
    float4 oColor0 : SV_TARGET;
};

struct CopyPushConstant
{
    int SourceTextureID;
    int Source2TextureID;
    int Mode;
};

[[vk::push_constant]]
CopyPushConstant pushConstants;

PSOutput main(PSInput input)
{
    PSOutput output;

    int sourceTextureID = pushConstants.SourceTextureID;
    int source2TextureID = pushConstants.Source2TextureID;

    float2 uv = input.UV;
    float4 sampleValue = textures[sourceTextureID].Sample(mySampler[0], input.UV);
    float4 sampleValue2 = textures[source2TextureID].Sample(mySampler[0], input.UV);

    if(pushConstants.Mode == 0)
    {
        output.oColor0 = lerp(sampleValue, sampleValue2, 1.0 - sampleValue.a);
    }
    else if(pushConstants.Mode == 1)
    {
        output.oColor0 = sampleValue;
        output.oColor0 = sampleValue + sampleValue2;
    }
    return output;
}
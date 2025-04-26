#include "Utils/header.hlsl"

struct PSInput 
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
};

struct PSOutput 
{
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
        float4 textureSample = textures[pushConstants.TextureID].Sample(mySampler[0], uv);

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
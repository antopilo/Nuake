#include "Utils/header.hlsl"

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
    return textures[textureId].Sample(mySampler[0], uv).rgb;
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
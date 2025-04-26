#include "Utils/header.hlsl"

struct PSInput 
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
};

struct PSOutput 
{
    float4 oColor0 : SV_TARGET;
};

struct LineConstant
{
    float4x4 Transform;
    float4 Color;
};

[[vk::push_constant]]
LineConstant pushConstants;

PSOutput main(PSInput input)
{
    PSOutput output;
    
    output.oColor0 = pushConstants.Color;

    return output;
}
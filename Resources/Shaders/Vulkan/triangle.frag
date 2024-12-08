struct PSInput {
    float3 Color : TEXCOORD0;
};

float4 main(PSInput input) : SV_Target 
{
    // Return color with alpha = 1.0f
    return float4(input.Color, 1.0f);
}
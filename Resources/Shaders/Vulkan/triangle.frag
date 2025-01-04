[[vk::binding(0, 3)]]
Texture2D<float3> myTexture : register(t1); // Texture binding at slot t0

[[vk::binding(0, 4)]]
SamplerState mySampler : register(s0);       // Sampler binding at slot s0

struct PSInput {
    float4 Position : SV_Position;
    float3 Color : TEXCOORD0;
    float2 UV : TEXCOORD1;
};

float4 main(PSInput input) : SV_Target 
{
    float3 textureColor = myTexture.Sample(mySampler, input.UV);
    // Return color with alpha = 1.0f
    return float4(textureColor, 1.0f);
}
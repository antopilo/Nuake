[[vk::binding(0, 3)]]
Texture2D<float3> myTexture : register(t1); // Texture binding at slot t0

[[vk::binding(0, 4)]]
SamplerState mySampler : register(s0);       // Sampler binding at slot s0

struct PSInput {
    float4 Position : SV_Position;
    float3 Color : TEXCOORD0;
    float2 UV : TEXCOORD1;
    float3 Normal : TEXCOORD2;
};

struct PSOutput {
    float4 oColor0 : SV_TARGET;
    float4 oNormal : SV_TARGET1;
};

PSOutput main(PSInput input)
{
    PSOutput output;
    float3 textureColor = myTexture.Sample(mySampler, input.UV);
    output.oColor0 = float4(textureColor, 1.0f);
    output.oNormal = float4(input.Normal, 1.0f);
    return output;
}
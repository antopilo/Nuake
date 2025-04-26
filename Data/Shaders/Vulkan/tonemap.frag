#include "Utils/header.hlsl"

struct PSInput {
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
};

struct PSOutput {
    float4 oColor0 : SV_TARGET;
};

struct TonemapPushConstant
{
    float Exposure;
    float Gamma;
    int SourceTextureID;
};

[[vk::push_constant]]
TonemapPushConstant pushConstants;

float3 PBRNeutralToneMapping(float3 color)
{
  const float startCompression = 0.8 - 0.04;
  const float desaturation = 0.15;

  float x = min(color.r, min(color.g, color.b));
  float offset = x < 0.08 ? x - 6.25 * x * x : 0.04;
  color -= offset;

  float peak = max(color.r, max(color.g, color.b));
  if (peak < startCompression) return color;

  const float d = 1. - startCompression;
  float newPeak = 1. - d * d / (peak + d - startCompression);
  color *= newPeak / peak;

  float g = 1. - 1. / (desaturation * (peak - newPeak) + 1.);
  return lerp(color, newPeak * float3(1, 1, 1), g);
}

PSOutput main(PSInput input)
{
    PSOutput output;
    float3 color = textures[pushConstants.SourceTextureID].Sample(mySampler[0], input.UV).rgb;
    float3 mapped = float3(1.0, 1.0, 1.0) - exp(-color * pushConstants.Exposure);

    color = pow(mapped, float3(pushConstants.Gamma, pushConstants.Gamma, pushConstants.Gamma));

    output.oColor0 = float4(color, 1);
    return output;
}
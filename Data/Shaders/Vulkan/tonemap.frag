// Transforms
struct ModelData
{
    float4x4 model;
};
[[vk::binding(0, 0)]]
StructuredBuffer<ModelData> model : register(t1);

// Vertex
struct Vertex 
{
    float3 position;
    float uv_x;
    float3 normal;
    float uv_y;
    float3 tangent;
    float3 bitangent;
};

[[vk::binding(0, 1)]] 
StructuredBuffer<Vertex> vertexBuffer : register(t2);

// Samplers
[[vk::binding(0, 2)]]
SamplerState mySampler[2] : register(s0);

// Materials
struct Material
{
    bool hasAlbedo;
    float3 albedo;
    bool hasNormal;
    bool hasMetalness;
    bool hasRoughness;
    bool hasAO;
    float metalnessValue;
    float roughnessValue;
    float aoValue;
    int albedoTextureId;
    int normalTextureId;
    int metalnessTextureId;
    int roughnessTextureId;
    int aoTextureId;
    int samplingType;
};
[[vk::binding(0, 3)]]
StructuredBuffer<Material> material;

// Textures
[[vk::binding(0, 4)]]
Texture2D textures[];

// Lights
struct Light
{
    float3 position;
    int type;
    float4 color;
    float3 direction;
    float outerConeAngle;
    float innerConeAngle;
    bool castShadow;
    int shadowMapTextureId[4];
    int transformId[4];
};

[[vk::binding(0, 5)]]
StructuredBuffer<Light> lights;

// Cameras
struct CameraView {
    float4x4 View;
    float4x4 Projection;
    float4x4 ViewProjection;
    float4x4 InverseView;
    float4x4 InverseProjection;
    float3 Position;
    float Near;
    float Far;
};
[[vk::binding(0, 6)]]
StructuredBuffer<CameraView> cameras;

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
    float3 color = textures[pushConstants.SourceTextureID].Sample(mySampler[1], input.UV).rgb;
    float3 mapped = float3(1.0, 1.0, 1.0) - exp(-color * pushConstants.Exposure);

    color = pow(mapped, float3(pushConstants.Gamma, pushConstants.Gamma, pushConstants.Gamma));

    output.oColor0 = float4(color, 1);
    return output;
}
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
SamplerState mySampler : register(s0);

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

struct OutlinePushConstant
{
    float4 Color;
    float Thickness;
    int SourceTextureID;
    int EntityIDTextureID;
    int DepthTextureID;
    float SelectedEntity;
};

[[vk::push_constant]]
OutlinePushConstant pushConstants;

PSOutput main(PSInput input)
{
    PSOutput output;

    float4 outlineColor = pushConstants.Color;
    float target = pushConstants.SelectedEntity;
    float radius = pushConstants.Thickness;
	float2 uv = input.UV;

    int entityIDTextureID = pushConstants.EntityIDTextureID;
    float hasHit = 0.0f;

    float sampleValue = textures[entityIDTextureID].Sample(mySampler, uv).r;

    if(abs(sampleValue - target) < 0.0001)
    {
        output.oColor0 = outlineColor;
        output.oColor0.a = 1.0;
        return output;
    }

    float4 fragColor = float4(0, 0, 0, 0);
    const float TAU = 6.28318530;
	const float steps = 32.0;
    for(float i = 0.0f; i < TAU; i += TAU / steps)
    {
        float2 uvOffset = float2(sin(i), cos(i)) * radius;

        float2 sampleUV = uv + uvOffset;
        sampleUV.x = clamp(sampleUV.x, 0.0, 0.999);
		sampleUV.y = clamp(sampleUV.y, 0.0, 0.999);
        
        int sampleValue = (int)textures[entityIDTextureID].Sample(mySampler, sampleUV).r;
        if(sampleValue == target)
        {
            hasHit = 1.0f;
        }

        float alpha = smoothstep(0.5, 0.9, int(sampleValue != target) * hasHit * 10.0f);
        float4 outputColor = float4(
			lerp(fragColor.r, outlineColor.r, alpha),
			lerp(fragColor.g, outlineColor.g, alpha),
			lerp(fragColor.b, outlineColor.b, alpha),
			lerp(fragColor.a, outlineColor.a, alpha)
		);

        fragColor = outputColor;
    }

    if(fragColor.a > 0.1)
    {
        fragColor.a = 1.0f;
    }

    output.oColor0 = fragColor;
    return output;
}
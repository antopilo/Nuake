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

struct PSInput 
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
};

struct PSOutput {
    float4 oColor0 : SV_TARGET;
};

struct VolumetricConstant
{
    int DepthTextureID;
    int StepCount;
    float FogAmount;
    float Exponant;
    int CamViewID;
    int LightCount;
    float Ambient;
};

[[vk::push_constant]]
VolumetricConstant pushConstants;

float2 GetTexelSize(Texture2D tex)
{
    uint width, height;
    tex.GetDimensions(width, height);
    return 1.0 / float2(width, height);
}

float LinearizeDepth(float depth, float nearPlane, float farPlane)
{
    return (2.0 * nearPlane) / (farPlane + nearPlane - (1.0 - depth) * (farPlane - nearPlane));
}

float ComputeScattering(float lightDotView)
{
    float PI = 3.141592653589793f;
    float result = 1.0f - pushConstants.FogAmount;
    result /= (4.0f * PI * pow(1.0f + pushConstants.FogAmount * pushConstants.FogAmount - (1.0f * pushConstants.FogAmount) * lightDotView, 1.5f));
    return result;
}

float3 WorldPosFromDepth(float depth, float2 uv, float4x4 invProj, float4x4 invView)
{
    float z = depth;
    float4 clipSpacePosition = float4(uv.x * 2.0 - 1.0, (uv.y * 2.0 - 1.0), z, 1.0f);
    float4 viewSpacePosition = mul(invProj, clipSpacePosition);
    viewSpacePosition /= viewSpacePosition.w;

    float4 worldSpacePosition = mul(invView, viewSpacePosition);
    return worldSpacePosition.xyz;
}

PSOutput main(PSInput input)
{
    CameraView camView = cameras[pushConstants.CamViewID];
    float3 startPosition = camView.Position;

    int depthTexture = pushConstants.DepthTextureID;
    float depth = textures[depthTexture].Sample(mySampler, input.UV).r;

    float3 worldPos = WorldPosFromDepth(depth, input.UV, camView.InverseProjection, camView.InverseView);

    float3 rayVector = worldPos - startPosition;

    float rayLength = length(rayVector);

    PSOutput output;
    if(rayLength > 1000.0)
    {
        output.oColor0 = float4(0.0f, 0.0f, 0, 0.0f);
        return output;
    }

    float stepLength = rayLength / pushConstants.StepCount;
    float3 rayDirection = rayVector / rayLength;
    float3 step = rayDirection * stepLength;

    float3 accumFog = float3(0, 0, 0);
    float3 currentPosition = startPosition;
    for(int i = 0; i < pushConstants.StepCount; i++)
    {
        for(int l = 0; l < pushConstants.LightCount; l++)
        {
            Light light = lights[l];
            if(light.type != 0) 
            {
                continue;
            }

            CameraView lightView = cameras[light.transformId[0]];
            float4 fragPosLightSpace = mul(lightView.Projection, mul(lightView.View, float4(currentPosition, 1.0)));
            float3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
            projCoords.xy = projCoords.xy * 0.5 + 0.5;

            float currentDepth = projCoords.z;
            float closestDepth = textures[light.shadowMapTextureId[0]].Sample(mySampler, projCoords.xy).r;

            if(closestDepth < currentDepth)
            {
                accumFog += (ComputeScattering(dot(rayDirection, light.direction)).rrr * light.color) * pushConstants.Exponant;
            }
            else
            {
                accumFog += (ComputeScattering(dot(rayDirection, light.direction)).rrr * light.color) * pushConstants.Ambient;
            }
        }

        currentPosition += step;
        
    }

    accumFog /= pushConstants.StepCount;
    
    output.oColor0 = float4(accumFog.x, accumFog.y, accumFog.z, 1.0f);
    return output;
}
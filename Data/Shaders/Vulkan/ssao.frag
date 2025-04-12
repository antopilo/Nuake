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

[[vk::binding(0, 7)]]
StructuredBuffer<float3> ssaoKernels;

struct PSInput {
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
};

struct PSOutput {
    float4 oColor0 : SV_TARGET;
};

struct SSAOConstant
{
    int noiseTextureID;
    int normalTextureID;
    int depthTextureID;
    int camViewID;
    float radius;
    float bias;
    float2 noiseScale;
    float power;
};

[[vk::push_constant]]
SSAOConstant pushConstants;

float3 ViewPosFromDepth(float depth, float2 uv, float4x4 invProj)
{
    float z = depth;
    float4 clipSpacePosition = float4(uv.x * 2.0 - 1.0, (uv.y * 2.0 - 1.0), z, 1.0f);
    float4 viewSpacePosition = mul(invProj, clipSpacePosition);
    viewSpacePosition /= viewSpacePosition.w;
    return viewSpacePosition.xyz;
}

float3 WorldPosFromDepth(float depth, float2 uv, CameraView camera)
{
    float z = depth;
    float4 clipSpacePosition = float4(uv.x * 2.0 - 1.0, (uv.y * 2.0 - 1.0), z, 1.0f);
    float4 viewSpacePosition = mul(camera.InverseProjection, clipSpacePosition);
    viewSpacePosition /= viewSpacePosition.w;
    
    float4 worldSpacePosition = mul(camera.InverseView, viewSpacePosition);
    worldSpacePosition /= worldSpacePosition.w;

    return worldSpacePosition.xyz;
}

float3 SampleTexture(int textureId, float2 uv)
{
    return textures[textureId].Sample(mySampler, uv).rgb;
}

float SampleDepth(float2 uv)
{
    return textures[pushConstants.depthTextureID].Sample(mySampler, uv).r;
}

float3x3 Inverse3x3(float3x3 m)
{
    float3 r0 = cross(m[1], m[2]);
    float3 r1 = cross(m[2], m[0]);
    float3 r2 = cross(m[0], m[1]);

    float det = dot(r2, m[2]);
    float invDet = 1.0 / det;

    return float3x3(
        r0 * invDet,
        r1 * invDet,
        r2 * invDet
    );
}

PSOutput main(PSInput input)
{
    CameraView camera = cameras[pushConstants.camViewID];

    float depth = SampleDepth(input.UV);

    // Discard the sky
    if(depth < 0.00001)
    {
        discard;
    }

    // Calculate TBN
    float3x3 normalMatrix = transpose(Inverse3x3((float3x3)camera.View));
    float3 normal = SampleTexture(pushConstants.normalTextureID, input.UV).xyz * 2.0 - 1.0;
    normal = mul(normalMatrix, normal);

    float3 randomVec = SampleTexture(pushConstants.noiseTextureID, input.UV * pushConstants.noiseScale).xyz;

    float3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    float3 bitangent = cross(tangent, normal);
    float3x3 TBN = float3x3(tangent, bitangent, normal);

    float3 fragPos = ViewPosFromDepth(depth, input.UV, camera.InverseProjection);
    // Fix from: https://github.com/JoeyDeVries/LearnOpenGL/issues/364
    float4 fragWorldPos = mul(camera.InverseView, float4(fragPos, 1.0));
    fragWorldPos.xyz /= fragWorldPos.w;

    float occlusion = 0.0f;
    for(int i = 0; i < 64; i++)
    {
        float3 samplePos = mul(TBN, ssaoKernels[i]);
        samplePos = fragWorldPos + samplePos * pushConstants.radius;

        // Fix from: https://github.com/JoeyDeVries/LearnOpenGL/issues/364
        float4 worldSamplePos = mul(camera.View, float4(samplePos, 1.0));
        worldSamplePos.xyz /= worldSamplePos.w;

        samplePos = worldSamplePos.xyz;

        float4 offset = float4(samplePos, 1.0f);
        offset = mul(camera.Projection, offset);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        float sampleDepth = ViewPosFromDepth(SampleDepth(offset.xy), offset.xy, camera.InverseProjection).z;
        float rangeCheck = smoothstep(0.0, 1.0, pushConstants.radius / abs(sampleDepth - fragPos.z));
        occlusion += (sampleDepth - pushConstants.bias >= samplePos.z  ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / 64.0);
    occlusion = pow(occlusion, pushConstants.power);

    PSOutput output;
    output.oColor0 = float4(occlusion, occlusion, occlusion, 1.0f);
    return output;
}
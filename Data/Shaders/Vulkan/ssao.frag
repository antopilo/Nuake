#include "Utils/header.hlsl"

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
    return textures[textureId].Sample(mySampler[0], uv).rgb;
}

float SampleDepth(float2 uv)
{
    return textures[pushConstants.depthTextureID].Sample(mySampler[0], uv).r;
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
    float3x3 normalMatrix = (float3x3)camera.View;
    //normalMatrix[0] *= -1.0; // Flip the Z basis vector
    float3 normal = SampleTexture(pushConstants.normalTextureID, input.UV).xyz * 2.0 - 1.0;
    normal = mul(normalMatrix, normal);

    float2 randomVecSample = SampleTexture(pushConstants.noiseTextureID, input.UV * pushConstants.noiseScale).xy * 2.0 - 1.0;
    float3 randomVec = float3(randomVecSample.x, -randomVecSample.y, 0);
    //randomVec = float3(0, 1, 0);
    float3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    float3 bitangent = cross(normal, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, normal);

    float3 fragPos = ViewPosFromDepth(depth, input.UV, camera.InverseProjection);
    // Fix from: https://github.com/JoeyDeVries/LearnOpenGL/issues/364
    float4 fragWorldPos = mul(camera.InverseView, float4(fragPos, 1.0));
    fragWorldPos.xyz /= fragWorldPos.w;

    PSOutput output;

    float occlusion = 0.0f;
    for(int i = 0; i < 64; i++)
    {
        float3 samplePos = mul(ssaoKernels[i], TBN);

        samplePos = fragPos + samplePos * pushConstants.radius;
        //samplePos = fragWorldPos + samplePos * pushConstants.radius;

        //return output;
        // Fix from: https://github.com/JoeyDeVries/LearnOpenGL/issues/364
        float4 worldSamplePos = mul(camera.View, float4(samplePos, 1.0));
        worldSamplePos.xyz /= worldSamplePos.w;

        //samplePos = worldSamplePos.xyz;
        float4 offset = float4(samplePos, 1.0f);
        offset = mul(camera.Projection, offset);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        offset.x = clamp(offset.x, 0.00001, 0.999);
        offset.y = clamp(offset.y, 0.00001, 0.999);

        float sampleDepth = ViewPosFromDepth(SampleDepth(offset.xy), offset.xy, camera.InverseProjection).z;
        float rangeCheck = smoothstep(0.0, 1.0, pushConstants.radius / abs(sampleDepth - fragPos.z));
        occlusion += (sampleDepth - pushConstants.bias >= samplePos.z  ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / 64.0);
    occlusion = pow(occlusion, pushConstants.power);

   
    output.oColor0 = float4(occlusion, occlusion, occlusion, 1.0f);
    return output;
}
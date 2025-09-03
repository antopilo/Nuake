#include "Utils/header.hlsl"

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
    float Time;
    float NoiseSpeed;
    float NoiseScale;
    float NoiseStrength;
    float CSMSplits[4];
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

// Simplex 3D Noise 
float mod289(float x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
float3 mod289(float3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
float4 mod289(float4 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }

float4 permute(float4 x) { return mod289(((x*34.0)+1.0)*x); }

float4 taylorInvSqrt(float4 r) { return 1.79284291400159 - 0.85373472095314 * r; }

float snoise(float3 v)
{
    const float2  C = float2(1.0/6.0, 1.0/3.0) ;
    const float4  D = float4(0.0, 0.5, 1.0, 2.0);

    // First corner
    float3 i  = floor(v + dot(v, C.yyy));
    float3 x0 = v - i + dot(i, C.xxx);

    // Other corners
    float3 g = step(x0.yzx, x0.xyz);
    float3 l = 1.0 - g;
    float3 i1 = min(g.xyz, l.zxy);
    float3 i2 = max(g.xyz, l.zxy);

    // x0 = x0 - 0.0 + 0.0 * C.xxx;
    float3 x1 = x0 - i1 + C.xxx;
    float3 x2 = x0 - i2 + C.yyy;
    float3 x3 = x0 - 1.0 + 3.0 * C.xxx;

    // Permutations
    i = mod289(i);
    float4 p = permute(permute(permute(
                i.z + float4(0.0, i1.z, i2.z, 1.0))
              + i.y + float4(0.0, i1.y, i2.y, 1.0))
              + i.x + float4(0.0, i1.x, i2.x, 1.0));

    // Gradients: 7x7 points over a cube, mapped onto a unit sphere
    float4 j = p - 49.0 * floor(p * (1.0 / 49.0));  // mod(p,7*7)

    float4 x_ = floor(j * (1.0 / 7.0));
    float4 y_ = floor(j - 7.0 * x_);  // mod(j,7)

    float4 x = (x_ * 2.0 + 0.5) / 7.0 - 1.0;
    float4 y = (y_ * 2.0 + 0.5) / 7.0 - 1.0;

    float4 h = 1.0 - abs(x) - abs(y);

    float4 b0 = float4(x.xy, y.xy);
    float4 b1 = float4(x.zw, y.zw);

    float4 s0 = floor(b0) * 2.0 + 1.0;
    float4 s1 = floor(b1) * 2.0 + 1.0;
    float4 sh = -step(h, 0.0);

    float4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
    float4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

    float3 g0 = float3(a0.xy, h.x);
    float3 g1 = float3(a0.zw, h.y);
    float3 g2 = float3(a1.xy, h.z);
    float3 g3 = float3(a1.zw, h.w);

    // Normalize gradients
    float4 norm = taylorInvSqrt(float4(dot(g0,g0), dot(g1,g1), dot(g2,g2), dot(g3,g3)));
    g0 *= norm.x;
    g1 *= norm.y;
    g2 *= norm.z;
    g3 *= norm.w;

    // Mix final noise value
    float4 m = max(0.6 - float4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
    m = m * m;
    return 42.0 * dot(m*m, float4(dot(g0,x0), dot(g1,x1), dot(g2,x2), dot(g3,x3)));
}

int GetCSMSplit(float depth)
{
    for(int i = 0; i < 4; i++)
    {
        float csmSplitDepth = pushConstants.CSMSplits[i];

        if(depth < csmSplitDepth + 0.000001)
        {
            return i;
        }
    }

    return 0;
}

PSOutput main(PSInput input)
{
    float ditherPattern[4][4] = { { 0.0f, 0.5f, 0.125f, 0.625f},
    { 0.75f, 0.22f, 0.875f, 0.375f},
    { 0.1875f, 0.6875f, 0.0625f, 0.5625},
    { 0.9375f, 0.4375f, 0.8125f, 0.3125} };

    CameraView camView = cameras[pushConstants.CamViewID];
    float3 startPosition = camView.Position;

    int depthTexture = pushConstants.DepthTextureID;
    float depth = textures[depthTexture].Sample(mySampler[0], input.UV).r;

    float3 worldPos = WorldPosFromDepth(depth, input.UV, camView.InverseProjection, camView.InverseView);

    float3 rayVector = worldPos - startPosition;

    float rayLength = length(rayVector);

    PSOutput output;
    if(rayLength > 1000.0)
    {
        output.oColor0 = float4(0.0f, 0.0f, 0, 0.0f);
        //return output;
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
            if(light.type == 0) 
            {
                float lightDepth = length(worldPos - camView.Position);
                int splitIndex = GetCSMSplit(lightDepth);

                if(splitIndex == -1)
                {
                    //accumFog += (ComputeScattering(dot(rayDirection, light.direction)).rrr * light.color.xyz) * pushConstants.Exponant;
                }
                else
                {
                    CameraView lightView = cameras[light.transformId[splitIndex]];
                    float4 fragPosLightSpace = mul(lightView.Projection, mul(lightView.View, float4(currentPosition, 1.0)));
                    float3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
                    projCoords.xy = projCoords.xy * 0.5 + 0.5;

                    float currentDepth = projCoords.z;
                    float closestDepth = textures[light.shadowMapTextureId[splitIndex]].Sample(mySampler[0], projCoords.xy).r;

                    float3 noiseOffset = float3(pushConstants.NoiseSpeed * pushConstants.Time, pushConstants.NoiseSpeed * pushConstants.Time, pushConstants.NoiseSpeed * pushConstants.Time);
                    float3 noiseSamplePos = (currentPosition + noiseOffset) * pushConstants.NoiseScale;
                    if(closestDepth < currentDepth)
                    {
                        accumFog += (ComputeScattering(dot(rayDirection, light.direction)).rrr * light.color.xyz) * pushConstants.Exponant * ((snoise(noiseSamplePos.xyz) + 1.0) / 2.0);
                    }
                    else
                    {
                        accumFog += (ComputeScattering(dot(rayDirection, light.direction)).rrr * light.color.xyz) * pushConstants.Ambient * ((snoise(noiseSamplePos.xyz) + 1.0) / 2.0);
                    }
                }


            }
            else if(light.type == 1)
            {
                float3 lightToFrag = currentPosition - light.position;
                float distance = length(lightToFrag);
                float3 lightDir = normalize(-lightToFrag);
                float attenuation = 1.0 / (distance * distance);
                attenuation = 1.0 - smoothstep(0.0, 3.0f, distance);
                float3 noiseOffset = float3(pushConstants.NoiseSpeed * pushConstants.Time, pushConstants.NoiseSpeed * pushConstants.Time, pushConstants.NoiseSpeed * pushConstants.Time);
                float3 noiseSamplePos = (currentPosition + noiseOffset) * pushConstants.NoiseScale;
                float lightScatter = (snoise(noiseSamplePos.xyz) + 1.0) * 0.5;

                float3 scatterTerm = ComputeScattering(dot(rayDirection, lightDir)).rrr * light.color.xyz;

                accumFog += scatterTerm * lightScatter * pushConstants.Exponant * attenuation;
            }
            else if(light.type == 2)
            {
                float3 lightToFrag = currentPosition - light.position;
                float distance = length(lightToFrag);
                float3 lightDir = normalize(-lightToFrag);
                float attenuation = 1.0 / (distance * distance);
                attenuation = 1.0 - smoothstep(0.0, 6.0f, distance);
                float3 noiseOffset = float3(pushConstants.NoiseSpeed * pushConstants.Time, pushConstants.NoiseSpeed * pushConstants.Time, pushConstants.NoiseSpeed * pushConstants.Time);
                float3 noiseSamplePos = (currentPosition + noiseOffset) * pushConstants.NoiseScale;
                float lightScatter = (snoise(noiseSamplePos.xyz) + 1.0) * 0.5;

                float theta = dot(lightDir, normalize(-light.direction));
                float epsilon = light.innerConeAngle - light.outerConeAngle;
                float intensity = clamp((theta - light.outerConeAngle) / epsilon, 0.0, 1.0);
                float3 scatterTerm = ComputeScattering(dot(rayDirection, lightDir)).rrr * light.color.xyz;
                accumFog += scatterTerm * lightScatter * pushConstants.Exponant * attenuation * intensity;
            }
        }



        currentPosition += step ;
        
    }

    accumFog /= pushConstants.StepCount;
    
    output.oColor0 = float4(accumFog.x, accumFog.y, accumFog.z, 1.0f);
    return output;
}
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
    int receiveShadow;
    int castShadow;
    int unlit;
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

struct ShadingPushConstant
{
    int AlbedoInputTextureId;
    int DepthInputTextureId;
    int NormalInputTextureId;
    int MaterialInputTextureId;
    int LightOffset;
    int LightCount;
    int CameraID;
    float AmbientTerm;
    float cascadeDepth[4];
    int SSAOTextureId;
    int EntityTextureId;
};

[[vk::push_constant]]
ShadingPushConstant pushConstants;

float3 WorldPosFromDepth(float depth, float2 uv, float4x4 invProj, float4x4 invView)
{
    float z = depth;
    float4 clipSpacePosition = float4(uv.x * 2.0 - 1.0, (uv.y * 2.0 - 1.0), z, 1.0f);
    float4 viewSpacePosition = mul(invProj, clipSpacePosition);
    viewSpacePosition /= viewSpacePosition.w;

    float4 worldSpacePosition = mul(invView, viewSpacePosition);
    return worldSpacePosition.xyz;
}

float LinearizeDepth(float depth, float nearPlane, float farPlane, bool reverseDepth)
{
    if (reverseDepth)
    {
        // Reverse depth (near plane = 1.0, far plane = 0.0)
        return nearPlane * farPlane / lerp(farPlane, nearPlane, depth);
    }
    else
    {
        // Standard depth (near plane = 0.0, far plane = 1.0)
        return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - depth * (farPlane - nearPlane));
    }
}

float DistributionGGX(float3 N, float3 H, float a)
{
    float PI = 3.141592653589793f;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);

    return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    float roughnessTerm = 1.0f - roughness;
    return F0 + (max(float3(roughnessTerm, roughnessTerm, roughnessTerm), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

float linearDepth(float z, float near, float far) {
    return near * far / (far - z * (far - near));
}

int GetCSMSplit(float depth)
{
    for(int i = 0; i < 4; i++)
    {
        float csmSplitDepth = pushConstants.cascadeDepth[i];

        if(depth < csmSplitDepth + 0.000001)
        {
            return i;
        }
    }

    return 0;
}

float SampleShadowMap(int textureId, float2 coords, float compare)
{
    return compare > textures[textureId].Sample(mySampler[0], coords.xy).r;
}

float SampleShadowMapLinear(int textureId, float2 coords, float compare, float2 texelSize)
{
    float2 pixelPos = coords / texelSize + float2(0.5f, 0.5f);
    float2 fracPart = frac(pixelPos);
    float2 startTexel = (pixelPos - fracPart) * texelSize;

    float blTexel = SampleShadowMap(textureId, startTexel, compare);
    float brTexel = SampleShadowMap(textureId, startTexel + float2(texelSize.x, 0.0), compare);
    float tlTexel = SampleShadowMap(textureId, startTexel + float2(0.0, texelSize.y), compare);
    float trTexel = SampleShadowMap(textureId, startTexel + texelSize, compare);

    float mixA = lerp(blTexel, tlTexel, fracPart.y);
    float mixB = lerp(brTexel, trTexel, fracPart.y);

    return lerp(mixA, mixB, fracPart.x);
}

float ShadowCalculation(Light light, float3 fragPos, float3 normal)
{
    // Find correct CSM splits from depth
    CameraView camView = cameras[pushConstants.CameraID];
    float depth = length(fragPos - camView.Position);
    int splitIndex = GetCSMSplit(depth);

    // Calculate shadows for found split
    CameraView lightView = cameras[light.transformId[splitIndex]];
    int shadowMap = light.shadowMapTextureId[0];
    float4 fragLightSpace = mul(lightView.Projection, mul(lightView.View, float4(fragPos, 1.0)));
    float3 projCoords = fragLightSpace.xyz / fragLightSpace.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0) {
        return 1.0;
    }

    //projCoords.y = 1.0 - projCoords.y;
    float currentDepth = projCoords.z;
    float bias = max(0.005 * (1.0 - dot(normal, light.direction)), 0.0005);

    if(splitIndex < 2)
    {
        const float NUM_SAMPLES = 4.0f;
        const float SAMPLES_START = (NUM_SAMPLES - 1.0f) / 2.0f;
        const float NUM_SAMPLES_SQUARED = NUM_SAMPLES * NUM_SAMPLES;

        float2 texelSize = 1.0f / float2(4096, 4096);
        float result = 0.0f;
        for(float y = -SAMPLES_START; y <= SAMPLES_START; y += 1.0f)
        {
            for (float x = -SAMPLES_START; x <= SAMPLES_START; x += 1.0f)
            {
                float2 coordsOffset = float2(x, y) * texelSize;
                result += SampleShadowMapLinear(light.shadowMapTextureId[splitIndex], projCoords.xy + coordsOffset, currentDepth, texelSize);
            }
        }

        return result /= NUM_SAMPLES_SQUARED;
    }

    float shadowMapDepth = textures[light.shadowMapTextureId[splitIndex]].Sample(mySampler[0], projCoords.xy).r;

    return (currentDepth > shadowMapDepth);//> 0.0 ? 1.0 : 0.0;
}
PSOutput main(PSInput input)
{
    PSOutput output;
    CameraView camView = cameras[pushConstants.CameraID];

    int depthTexture = pushConstants.DepthInputTextureId;
    float depth = textures[depthTexture].Sample(mySampler[0], input.UV).r;

    if(depth == 0.0f)
    {
        discard;
    }

    int albedoTextureId = pushConstants.AlbedoInputTextureId;
    float3 albedo = textures[albedoTextureId].Sample(mySampler[0], input.UV).xyz;
    int materialId = (int)textures[pushConstants.EntityTextureId].Sample(mySampler[0], input.UV).g;
    Material inMaterial = material[materialId];

    if(inMaterial.unlit)
    {
        output.oColor0 = float4(albedo, 1);
        return output;
    }

    float3 worldPos = WorldPosFromDepth(depth, input.UV, camView.InverseProjection, camView.InverseView);
   
    float3 normal = textures[pushConstants.NormalInputTextureId].Sample(mySampler[0], input.UV).rgb;
    normal = normal * 2.0f - 1.0f;

    float4 materialSample = textures[pushConstants.MaterialInputTextureId].Sample(mySampler[0], input.UV);
    float metallic = materialSample.r;
    float ao = materialSample.g;
    float roughness = materialSample.b;
    float ssao = textures[pushConstants.SSAOTextureId].Sample(mySampler[0], input.UV).r;



    float3 N = normal;
    float3 V = normalize(camView.Position - worldPos);
    float3 R = reflect(-V, N);
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);

    Light directionalLight;
    bool foundDirectional = false;
    for(int i = pushConstants.LightOffset; i < pushConstants.LightOffset + pushConstants.LightCount; i++)
    {
        Light light = lights[i];
        if(light.type == 0) 
        {
            directionalLight = light;
            foundDirectional = true;
            break;
        }
    }

    const float PI = 3.141592653589793f;
    float3 Lo = float3(0.0, 0.0, 0.0);
    float shadow = 1.0f;

    if(foundDirectional == false)
    {
        shadow = 1.0f;
    }

    //Directional
    if(foundDirectional)
    {
        Light light = directionalLight;
        float3 L = normalize(light.direction);
        float attenuation = 1.0f;

        if(light.castShadow == true)
        {
            shadow *= ShadowCalculation(light, worldPos, N);
            //output.oColor0 = float4(albedo * 0.1 + float3(shadow, shadow, shadow), 1);
            //return output;
        }

        // TODO: Shadow
        float3 radiance = light.color.rgb * attenuation;
        float3 H = normalize(V + L);
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        float3 nominator = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
        float3 specular = nominator / denominator;

        float3 kS = F;
        float3 kD = float3(1.0, 1.0, 1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);

        if(inMaterial.receiveShadow == 0)
        {
            shadow = 1.0f;
        }
        Lo += (kD * albedo / PI + specular) * radiance * NdotL * shadow;
    }

    // other lights
    for(int i = pushConstants.LightOffset; i < pushConstants.LightOffset + pushConstants.LightCount; i++)
    {
        Light light = lights[i];
        float3 L = normalize(light.position - worldPos);
        float distance = length(light.position - worldPos);
        float attenuation = 1.0 / (distance * distance);

        float3 radiance = float3(0, 0, 0);
        if(light.type == 1) // point light
        {
            radiance = light.color * attenuation;
        }
        else if(light.type == 2)
        {
            float theta = dot(L, normalize(-light.direction));
            float epsilon = light.innerConeAngle - light.outerConeAngle;
            float intensity = clamp((theta - light.outerConeAngle) / epsilon, 0.0, 1.0);
            radiance = light.color * intensity * attenuation;
        }

        float3 H = normalize(V + L);
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        float3 nominator = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
        float3 specular = nominator / denominator;

        float3 kS = F;
        float3 kD = float3(1.0, 1.0, 1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    float3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    float3 ambient = (albedo) * ao * ssao * pushConstants.AmbientTerm;
    float3 color = (ambient) + Lo;

    output.oColor0 = float4(color, 1);
    return output;
}
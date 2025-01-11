struct Camera
{
    float4x4 view;
    float4x4 proj;
    float4x4 invView;
    float4x4 invProj;
    float3 position;
};
[[vk::binding(0, 0)]]
StructuredBuffer<Camera> camera : register(t0);

[[vk::binding(0, 3)]]
SamplerState mySampler : register(s0);       // Sampler binding at slot s0

struct Material
{
    float hasAlbedo;
    float3 albedo;
    int hasNormal;
    int hasMetalness;
    int hasRoughness;
    int hasAO;
    float metalnessValue;
    float roughnessValue;
    float aoValue;
};
[[vk::binding(0, 4)]]
StructuredBuffer<Material> material;

[[vk::binding(0, 5)]]
Texture2D textures[]; // Array de 500 textures

struct Light
{
    float3 position;
    int type;
    float4 color;
    float3 direction;
    float outerConeAngle;
    float innerConeAngle;
    bool castShadow;
    int shadowMapTextureId;
    int transformId;
};

[[vk::binding(0, 6)]]
StructuredBuffer<Light> lights;


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
[[vk::binding(0, 7)]]
StructuredBuffer<CameraView> cameras;

struct PSInput {
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
    int LightCount;
    int CameraID;
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

PSOutput main(PSInput input)
{
    PSOutput output;
    Camera camData = camera[0];
    CameraView camView = cameras[pushConstants.CameraID];

    int depthTexture = pushConstants.DepthInputTextureId;
    float depth = textures[depthTexture].Sample(mySampler, input.UV).r;


    float3 worldPos = WorldPosFromDepth(depth, input.UV, camView.InverseProjection, camView.InverseView);
   
    int albedoTextureId = pushConstants.AlbedoInputTextureId;
    float3 albedo = textures[albedoTextureId].Sample(mySampler, input.UV).xyz;
    float3 normal = textures[pushConstants.NormalInputTextureId].Sample(mySampler, input.UV).rgb;
    normal = normal * 2.0f - 1.0f;

    float4 materialSample = textures[pushConstants.MaterialInputTextureId].Sample(mySampler, input.UV);
    float metallic = materialSample.r;
    float ao = materialSample.g;
    float roughness = materialSample.b;
    
    float3 N = normal;
    float3 V = normalize(camView.Position - worldPos);
    float3 R = reflect(-V, N);
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);

    Light directionalLight;
    bool foundDirectional = false;
    for(int i = 0; i < pushConstants.LightCount; i++)
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
    //Directional
    if(foundDirectional)
    {
        Light light = directionalLight;
        float3 L = normalize(light.direction);
        float attenuation = 1.0f;

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
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // other lights
    for(int i = 0; i < pushConstants.LightCount; i++)
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
            radiance = light.color * intensity;
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

    float3 ambient = (albedo) * ao  * 0.5f;
    float3 color = (ambient) + Lo;

    output.oColor0 = float4(color, 1);
    return output;
}
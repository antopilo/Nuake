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
    int shadowMapTextureId[4];
    int transformId[4];
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
    float cascadeDepth[4];
};

[[vk::push_constant]]
ShadingPushConstant pushConstants;

static const float2 poissonDisk64[64] =
{
    float2 ( 0.1187053,   0.7951565),
    float2 ( 0.1173675,   0.6087878),
    float2 (-0.09958518,  0.7248842),
    float2 ( 0.4259812,   0.6152718),
    float2 ( 0.3723574,   0.8892787),
    float2 (-0.02289676,  0.9972908),
    float2 (-0.08234791,  0.5048386),
    float2 ( 0.1821235,   0.9673787),
    float2 (-0.2137264,   0.9011746),
    float2 ( 0.3115066,   0.4205415),
    float2 ( 0.1216329,   0.383266),
    float2 ( 0.5948939,   0.7594361),
    float2 ( 0.7576465,   0.5336417),
    float2 (-0.521125,    0.7599803),
    float2 (-0.2923127,   0.6545699),
    float2 ( 0.6782473,   0.22385),
    float2 (-0.3077152,   0.4697627),
    float2 ( 0.4484913,   0.2619455),
    float2 (-0.5308799,   0.4998215),
    float2 (-0.7379634,   0.5304936),
    float2 ( 0.02613133,  0.1764302),
    float2 (-0.1461073,   0.3047384),
    float2 (-0.8451027,   0.3249073),
    float2 (-0.4507707,   0.2101997),
    float2 (-0.6137282,   0.3283674),
    float2 (-0.2385868,   0.08716244),
    float2 ( 0.3386548,   0.01528411),
    float2 (-0.04230833, -0.1494652),
    float2 ( 0.167115,   -0.1098648),
    float2 (-0.525606,    0.01572019),
    float2 (-0.7966855,   0.1318727),
    float2 ( 0.5704287,   0.4778273),
    float2 (-0.9516637,   0.002725032),
    float2 (-0.7068223,  -0.1572321),
    float2 ( 0.2173306,  -0.3494083),
    float2 ( 0.06100426, -0.4492816),
    float2 ( 0.2333982,   0.2247189),
    float2 ( 0.07270987, -0.6396734),
    float2 ( 0.4670808,  -0.2324669),
    float2 ( 0.3729528,  -0.512625),
    float2 ( 0.5675077,  -0.4054544),
    float2 (-0.3691984,  -0.128435),
    float2 ( 0.8752473,   0.2256988),
    float2 (-0.2680127,  -0.4684393),
    float2 (-0.1177551,  -0.7205751),
    float2 (-0.1270121,  -0.3105424),
    float2 ( 0.5595394,  -0.06309237),
    float2 (-0.9299136,  -0.1870008),
    float2 ( 0.974674,    0.03677348),
    float2 ( 0.7726735,  -0.06944724),
    float2 (-0.4995361,  -0.3663749),
    float2 ( 0.6474168,  -0.2315787),
    float2 ( 0.1911449,  -0.8858921),
    float2 ( 0.3671001,  -0.7970535),
    float2 (-0.6970353,  -0.4449432),
    float2 (-0.417599,   -0.7189326),
    float2 (-0.5584748,  -0.6026504),
    float2 (-0.02624448, -0.9141423),
    float2 ( 0.565636,   -0.6585149),
    float2 (-0.874976,   -0.3997879),
    float2 ( 0.9177843,  -0.2110524),
    float2 ( 0.8156927,  -0.3969557),
    float2 (-0.2833054,  -0.8395444),
    float2 ( 0.799141,   -0.5886372)
};

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

float ShadowCalculation(Light light, float3 fragPos, float3 normal)
{
    CameraView camView = cameras[pushConstants.CameraID];
    float depth = length(fragPos - camView.Position);

    CameraView lightView = cameras[light.transformId[0]];
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
    float shadowMapDepth = textures[light.shadowMapTextureId[0]].Sample(mySampler, projCoords.xy).r;

    return (currentDepth > shadowMapDepth);//> 0.0 ? 1.0 : 0.0;
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
        Lo += (kD * albedo / PI + specular) * radiance * NdotL * shadow;
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
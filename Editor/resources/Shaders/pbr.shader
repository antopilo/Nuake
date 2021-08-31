#shader vertex
#version 460 core
layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 UVPosition;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 Tangent;
layout(location = 4) in vec3 Bitangent;

out flat vec2 v_UVPosition;
out vec3 v_Normal;
out vec3 v_FragPos;
out vec3 v_ViewPos;
out mat3 v_TBN;
out vec3 v_Tangent;
out vec3 v_Bitangent;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;

void main()
{
    mat3 normalMatrix = mat3(transpose(inverse(u_Model)));
    v_Normal = vec3(vec4(Normal, 1.0f));

    vec3 N = normalize((u_Model * vec4(Normal, 0.0f)).xyz);
    vec3 T = normalize((u_Model * vec4(Tangent, 0.0f)).xyz);
    vec3 B = normalize((u_Model * vec4(Bitangent, 0.0f)).xyz);
    v_TBN = mat3(T, B, N);

    v_Tangent = T;
    v_Bitangent = B;

    v_UVPosition = UVPosition;

    gl_Position = u_Projection * u_View * u_Model * vec4(VertexPosition, 1.0f);
    v_FragPos = vec3(u_Model * vec4(VertexPosition, 1.0f));
    v_ViewPos = VertexPosition;
}

#shader fragment
#version 460 core

out vec4 FragColor;

in vec3 v_FragPos;
in vec3 v_ViewPos;
in vec2 v_UVPosition;
in flat vec3 v_Normal;
in mat3 v_TBN;
in vec3 v_Tangent;
in vec3 v_Bitangent;

const float PI = 3.141592653589793f;

struct Light {
    int Type; // 0 = directional, 1 = point
    vec3 Direction;
    vec3 Color;
    float Strength;
    vec3 Position;
    float ConstantAttenuation;
    float LinearAttenuation;
    float QuadraticAttenuation;
    mat4 LightTransform;
    sampler2D ShadowMaps[4];
    float CascadeDepth[4];
    mat4 LightTransforms[4];
    sampler2D ShadowMap;
    sampler2D RSMFlux;
    sampler2D RSMNormal;
    sampler2D RSMPos;
    int Volumetric;
};

// Debug
uniform int u_ShowNormal;

// Lights
const int MaxLight = 24;
uniform int LightCount = 0;
uniform Light Lights[MaxLight];

// Camera
uniform vec3 u_EyePosition;
uniform float u_Exposure;

// Environmnent
uniform float u_FogAmount;
uniform samplerCube u_IrradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D   brdfLUT;

// Material
uniform sampler2D m_Albedo; 
uniform sampler2D m_Metalness; 
uniform sampler2D m_Roughness; 
uniform sampler2D m_AO;
uniform sampler2D m_Normal;
uniform sampler2D m_Displacement;

layout(std140, binding = 32) uniform u_MaterialUniform
{
    int u_HasAlbedo; 
    vec3 m_AlbedoColor; //  16 byte
    int u_HasMetalness; //  32 byte
    float u_MetalnessValue; //  36 byte
    int u_HasRoughness; //  40 byte
    float u_RoughnessValue; //  44 byte
    int u_HasAO; //  48 byte
    float u_AOValue; //  52 byte
    int u_HasNormal; //  56 byte
    int u_HasDisplacement; //  60 byte
    int u_Unlit;
};

float height_scale = 0.00f;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir) // nice never done this // its easy Af its basicalyy returns a uv coords . that u use everywhere
{
    // number of depth layers
    const float minLayers = 8.0;
    const float maxLayers = 64.0;
    float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));

    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy * height_scale;
    vec2 deltaTexCoords = P / numLayers;

    // get initial values
    vec2  currentTexCoords = texCoords;
    float currentDepthMapValue = texture(m_Displacement, currentTexCoords).r;

    while (currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(m_Displacement, currentTexCoords).r;
        // get depth of next layer
        currentLayerDepth += layerDepth;
    }

    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(m_Displacement, prevTexCoords).r - currentLayerDepth + layerDepth;

    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;

    //float height = 1.0f - texture(m_Displacement, texCoords).r;
    //vec2 p = viewDir.xy / viewDir.z * (height * height_scale);
    //return texCoords - p;
}

float DistributionGGX(vec3 N, vec3 H, float a)
{
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

float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

float ShadowCalculation(Light light, vec3 FragPos, vec3 normal)
{
    // Get Depth
    vec3 startPosition = u_EyePosition;             
    vec3 rayVector = FragPos - startPosition;  
    float depth = length(rayVector);

    int shadowmap = -1;

    // Get CSM depth
    for (int i = 0; i < 4; i++)
    {
        float CSMDepth = light.CascadeDepth[i] ;

        if (depth < CSMDepth + 0.0001)
        {
            shadowmap = i;
            break;
        }
    }

    if (shadowmap == -1)
        return 1.0;

    vec4 fragPosLightSpace = light.LightTransforms[shadowmap] * vec4(FragPos, 1.0f);

    //sampler2D shadowmapFB = light.ShadowMaps[shadowmap];

    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(light.ShadowMaps[shadowmap], projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float bias = max(0.005 * (1.0 - dot(normal, light.Direction)), 0.0005);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(light.ShadowMaps[shadowmap], 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(light.ShadowMaps[shadowmap], projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    return shadow /= 9;
}

// Mie scaterring approximated with Henyey-Greenstein phase function.
float ComputeScattering(float lightDotView)
{
    float result = 1.0f - u_FogAmount * u_FogAmount;
    result /= (4.0f * PI * pow(1.0f + u_FogAmount * u_FogAmount - (2.0f * u_FogAmount) * lightDotView, 1.5f));
    return result;
}

uniform float u_FogStepCount;
vec3 ComputeVolumetric(vec3 FragPos, Light light)
{
    // world space frag position.
    vec3 startPosition = u_EyePosition;             // Camera Position
    vec3 rayVector = FragPos - startPosition;  // Ray Direction

    float rayLength = length(rayVector);            // Length of the raymarched

    float stepLength = rayLength / u_FogStepCount;        // Step length
    vec3 rayDirection = rayVector / rayLength;
    vec3 step = rayDirection * stepLength;          // Normalized to step length direction

    vec3 currentPosition = startPosition;           // First step position
    vec3 accumFog = vec3(0.0f, 0.0f, 0.0f);         // accumulative color

    // Raymarching
    for (int i = 0; i < u_FogStepCount; i++)
    {
        vec4 fragPosLightSpace = light.LightTransforms[0] * vec4(currentPosition, 1.0f);
        // perform perspective divide
        vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
        // transform to [0,1] range
        projCoords = projCoords * 0.5 + 0.5;

        float currentDepth = projCoords.z;

        // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
        vec2 texelSize = 1.0 / textureSize(light.ShadowMaps[0], 0);

        float closestDepth = texture(light.ShadowMaps[0], projCoords.xy).r;

        if (closestDepth > currentDepth)
            accumFog += (ComputeScattering(dot(rayDirection, light.Direction)).xxx * light.Color);

        currentPosition += step;
    }
    accumFog /= u_FogStepCount;

    return accumFog;
}

void main()
{
    // Parallax UV offset.
    vec3 tangentViewPos = v_TBN * u_EyePosition;
    vec3 tangentFragPos = v_TBN * v_FragPos;
    vec3 viewDir = normalize(tangentViewPos - tangentFragPos);
    vec2 texCoords = v_UVPosition;//ParallaxMapping(v_UVPosition, viewDir);
    vec2 finalTexCoords = texCoords;

    vec3 finalAlbedo = m_AlbedoColor;
    if(u_HasAlbedo == 1)
        finalAlbedo = texture(m_Albedo, finalTexCoords).rgb;

    float finalRoughness = u_RoughnessValue;
    if (u_HasRoughness == 1)
        finalRoughness = texture(m_Roughness, finalTexCoords).r;

    float finalMetalness = u_MetalnessValue;
    if (u_HasMetalness == 1)
        finalMetalness = texture(m_Metalness, finalTexCoords).r;

    float finalAO = u_AOValue;
    if (u_HasAO == 1)
        finalAO = texture(m_AO, finalTexCoords).r;

    vec3 finalNormal = vec3(0.5, 0.5, 1.0);
    if (u_HasNormal == 1)
    {
        finalNormal = texture(m_Normal, finalTexCoords).rgb;
    }

    finalNormal = finalNormal * 2.0 - 1.0;
    finalNormal = v_TBN * normalize(finalNormal);

    vec3 N = normalize(finalNormal);
    vec3 V = normalize(u_EyePosition - v_FragPos);
    vec3 R = reflect(-V, N);
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, finalAlbedo, finalMetalness);

    vec3 eyeDirection = normalize(u_EyePosition - v_FragPos);

    vec3 Fog = vec3(0.0);
    float shadow = 0.0f;
    vec3 Lo = vec3(0.0);

    if (u_Unlit == 1)
    {
        FragColor = vec4(finalAlbedo, 1.0);
        return;
    }

    for (int i = 0; i < LightCount; i++)
    {
        vec3 L = normalize(Lights[i].Position - v_FragPos);

        float distance = length(Lights[i].Position - v_FragPos);
        float attenuation = 1.0 / (distance * distance);    

        if (Lights[i].Type == 0) {
            L = normalize(Lights[i].Direction);
            attenuation = 1.0f;
            if (Lights[i].Volumetric == 1)
                Fog += ComputeVolumetric(v_FragPos, Lights[i]);

            shadow += ShadowCalculation(Lights[i], v_FragPos, N);
        }

       
        vec3 H = normalize(V + L);
        vec3 radiance = Lights[i].Color * attenuation * (1.0f - shadow);

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, finalRoughness);
        float G = GeometrySmith(N, V, L, finalRoughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 nominator = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
        vec3 specular = nominator / denominator;

        // kS is equal to Fresnel
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - finalMetalness;

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * finalAlbedo  / PI + specular ) * radiance * NdotL;// note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    /// ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, finalRoughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - finalMetalness;

    vec3 irradiance = mix(texture(u_IrradianceMap, N).rgb, vec3(0.1f), 0.9f);
    vec3 diffuse = irradiance * finalAlbedo;

    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R, finalRoughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), finalRoughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * (diffuse + specular)) * finalAO;
    vec3 color = ambient + Lo;

    color += Fog;
    // HDR tonemapping
    color = color / (color + vec3(1.0));


    const float gamma = 2.2;

    color = vec3(1.0) - exp(-color * u_Exposure);
    // gamma correct
    color = pow(color, vec3(1.0 / gamma));

    //ComputeVolumetric
    color = mix(color, finalNormal, u_ShowNormal);

    FragColor = vec4(color, 1.0); // so If i wanted to implement other stuff like SSR and bloom. I would need another render texture? using this same shader?
}
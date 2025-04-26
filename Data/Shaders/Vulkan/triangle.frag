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
    float4 tangent;
    float4 bitangent;
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
    float3 Color : TEXCOORD0;
    float2 UV : TEXCOORD1;
    float3 Normal : TEXCOORD2;
    float3 Tangent : TEXCOORD3;
    float3 Bitangent : TEXCOORD4;
};

struct PSOutput {
    float4 oColor0 : SV_TARGET;
    float4 oNormal : SV_TARGET1;
    float4 oMaterial : SV_TARGET2;
    float4 oEntityID : SV_TARGET3;
};

struct ModelPushConstant
{
    int modelIndex;  // Push constant data
    int materialIndex;
    int cameraID;
    float entityID;
};

[[vk::push_constant]]
ModelPushConstant pushConstants;



PSOutput main(PSInput input)
{
    PSOutput output;

    Material inMaterial = material[pushConstants.materialIndex]; 

    SamplerState samplerr = mySampler[inMaterial.samplingType];
    // NORMAL
    // TODO use TBN matrix
    float3 T = input.Tangent.xyz;
    float3 B = input.Bitangent.xyz;
    float3 N = input.Normal.xyz;
    float3x3 TBN = float3x3(T, B, N);

    float3 normal = float3(0.0, 0.0, 1.0);
    if(inMaterial.hasNormal == 1)
    {
        normal = textures[inMaterial.normalTextureId].Sample(samplerr, input.UV).rgb;
        normal.xyz = normal.zxy;
        normal = normal * 2.0f - 1.0f;
    }
    else
    {
        normal = normal;
    }

    //normal = input.Normal;
    normal = mul(transpose(TBN), normal);
    normal = normal / 2.0f + 0.5f;
    output.oNormal = float4(normal, 1.0f);

    // MATERIAL

    // ALBEDO COLOR
    float4 albedoColor = float4(inMaterial.albedo.xyz, 1.0f);
    if(inMaterial.hasAlbedo == 1)
    {
        float4 albedoSample = textures[inMaterial.albedoTextureId].Sample(samplerr, input.UV);

        // Alpha cutout?
        if(albedoSample.a < 0.001f)
        {
            discard;
        }

        albedoColor.xyz = albedoSample.xyz;
    }
    output.oColor0 = albedoColor;

    // MATERIAL PROPERTIES
    float metalnessValue = inMaterial.metalnessValue;
    if(inMaterial.hasMetalness == 1)
    {
        // TODO: Sample from metal texture
    }
    
    float aoValue = inMaterial.aoValue;
    if(inMaterial.hasAO == 1)
    {
        // TODO: Sample from AO texture
    }

    float roughnessValue = inMaterial.roughnessValue; 
    if(inMaterial.hasRoughness == 1)
    {
        // TODO: Sample from roughness texture
    }

    float3 materialOuput = float3(inMaterial.metalnessValue, inMaterial.aoValue, inMaterial.roughnessValue);
    
    output.oMaterial = float4(materialOuput, 1.0f);

    output.oEntityID = float4(pushConstants.entityID, pushConstants.materialIndex, 0.0f, 1.0f);

    return output;
}
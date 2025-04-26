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

struct ModelPushConstant
{
    int modelIndex;  // Push constant data
    int materialIndex;
    int cameraID;
    float entityID;
};

[[vk::push_constant]]
ModelPushConstant pushConstants;

// Outputs
struct VSOutput 
{
    float4 Position : SV_Position;
    float3 Color : TEXCOORD0;
    float2 UV : TEXCOORD1;
    float3 Normal : TEXCOORD2;
    float3 Tangent : TEXCOORD3;
    float3 Bitangent : TEXCOORD4;
};

float3x3 invert(float3x3 m)
{
    float3 a = m[0];
    float3 b = m[1];
    float3 c = m[2];

    float3 r0 = cross(b, c);
    float3 r1 = cross(c, a);
    float3 r2 = cross(a, b);

    float det = dot(r2, c);

    // Return identity if not invertible (optional fallback)
    if (abs(det) < 1e-6)
        return float3x3(1.0, 0.0, 0.0,
                        0.0, 1.0, 0.0,
                        0.0, 0.0, 1.0);

    float invDet = 1.0 / det;

    return float3x3(r0 * invDet,
                    r1 * invDet,
                    r2 * invDet);
}

// Main vertex shader
VSOutput main(uint vertexIndex : SV_VertexID) 
{
    VSOutput output;

    CameraView camView = cameras[pushConstants.cameraID];
    ModelData modelData = model[pushConstants.modelIndex];

    // Load vertex data from the buffer
    Vertex v = vertexBuffer[vertexIndex];

    // Output the position of each vertex
    output.Position = mul(camView.Projection, mul(camView.View, mul(modelData.model, float4(v.position, 1.0f))));
    output.Color = normalize(float3(v.position.xyz));
    output.UV = float2(v.uv_x, v.uv_y);
    
    float3x3 upper3x3 = (float3x3)modelData.model;
    float3x3 normalMatrix = transpose(invert(upper3x3));
    output.Bitangent = mul(normalMatrix, normalize(v.bitangent.xyz));
    output.Normal = mul(normalMatrix, normalize(v.normal.xyz));
    output.Tangent = mul(normalMatrix, normalize(v.tangent.xyz));

    //output.Normal = v.normal.xyz;
    return output;
}
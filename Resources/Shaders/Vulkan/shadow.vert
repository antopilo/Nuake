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

struct ModelData
{
    float4x4 model;
};
[[vk::binding(0, 1)]]
StructuredBuffer<ModelData> model : register(t1);

struct Vertex 
{
    float3 position;
    float uv_x;
    float3 normal;
    float uv_y;
    float3 tangent;
    float3 bitangent;
};

[[vk::binding(0, 2)]] 
StructuredBuffer<Vertex> vertexBuffer : register(t2);

struct ModelPushConstant
{
    int modelIndex;  // Push constant data
    int materialIndex;
    int cameraID;
};

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

[[vk::push_constant]]
ModelPushConstant pushConstants;

// Outputs
struct VSOutput {
    float4 Position : SV_Position;
};

// Main vertex shader
VSOutput main(uint vertexIndex : SV_VertexID) 
{
    VSOutput output;

    Camera camData = camera[0];
    ModelData modelData = model[pushConstants.modelIndex];

    CameraView camView = cameras[pushConstants.cameraID];

    // Load vertex data from the buffer
    Vertex v = vertexBuffer[vertexIndex];

    // Output the position of each vertex
    output.Position = mul(camView.Projection, mul(camView.View,mul(modelData.model, float4(v.position, 1.0f))));

    return output;
}
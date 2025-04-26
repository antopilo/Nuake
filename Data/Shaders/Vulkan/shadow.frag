#include "Utils/header.hlsl"

struct PSInput 
{
    float4 Position : SV_Position;
};

struct ModelPushConstant
{
    int modelIndex;  // Push constant data
    int materialIndex;
    int cameraID;
};

[[vk::push_constant]]
ModelPushConstant pushConstants;

void main(PSInput input)
{
}
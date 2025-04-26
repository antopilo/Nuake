#include "Utils/header.hlsl"

struct PSInput 
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
};

struct PSOutput {
    float4 oColor0 : SV_TARGET;
};

struct DepthAwareBlurConstant
{
    int DepthTextureID;
    int VolumetricTextureID;
};

[[vk::push_constant]]
DepthAwareBlurConstant pushConstants;

float2 GetTextureSize(Texture2D tex)
{
    uint width, height;
    tex.GetDimensions(width, height);
    return float2(width, height);
}

float PixelToUV(float2 uv, Texture2D tex)
{
    float2 texSize = GetTextureSize(tex);
    return uv / texSize;
}

PSOutput main(PSInput input)
{
    int depthTexture = pushConstants.DepthTextureID;
    float upSampledDepth = textures[depthTexture].Sample(mySampler[0], input.UV).r;
    float3 upSampledColor = textures[pushConstants.VolumetricTextureID].Sample(mySampler[0], input.UV).rgb;
    float3 color = 0.0f.xxx;
	float totalWeight = 0.0f;
	
    int2 screenCoordinates = int2(input.Position.xy);
    int xOffset = (screenCoordinates.x % 2 == 0) ? -1 : 1;
    int yOffset = (screenCoordinates.y % 2 == 0) ? -1 : 1;

	int2 offsets[] = {int2(0, 0),
	int2(0, yOffset),
	int2(xOffset, 0),
	int2(xOffset, yOffset)};
	
	for (int i = 0; i < 4; i ++)
	{
        float2 uvOffset = float2(offsets[i].x * 4.0, offsets[i].y * 4.0) ;
        uvOffset = PixelToUV(uvOffset, textures[pushConstants.DepthTextureID]);
        float3 downscaledColor = textures[pushConstants.VolumetricTextureID].Sample(mySampler[0], input.UV + uvOffset).rgb;
		float downscaledDepth = textures[pushConstants.DepthTextureID].Sample(mySampler[0], input.UV + uvOffset).r;
		
		float currentWeight = 1.0f;

        if(abs(upSampledDepth - downscaledDepth) > 0.0001)
        {
            //color = float3(1, 0, 0);
            currentWeight *= 0.0f;
        }
		//currentWeight *= max(0.0f, 1.0f - abs(upSampledDepth - downscaledDepth));
		
		color += downscaledColor * currentWeight;
		totalWeight += currentWeight;
	}
	
	float3 volumetricLight;
	const float epsilon = 0.0001f;
	volumetricLight.xyz = color / (totalWeight + epsilon);
	
    PSOutput output; 
    output.oColor0 = float4(volumetricLight.x, volumetricLight.y, volumetricLight.z, 1.0f);
    //output.oColor0 = float4(upSampledColor.x, upSampledColor.y, upSampledColor.z, 1.0f);
    return output;
}
#include "Utils/header.hlsl"

struct PSInput {
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
};

struct PSOutput {
    float4 oColor0 : SV_TARGET;
};

struct OutlinePushConstant
{
    float4 Color;
    float Thickness;
    int SourceTextureID;
    int EntityIDTextureID;
    int DepthTextureID;
    float SelectedEntity;
};

[[vk::push_constant]]
OutlinePushConstant pushConstants;

float2 GetTexelSize(Texture2D tex)
{
    uint width, height;
    tex.GetDimensions(width, height);
    return 1.0 / float2(width, height);
}

PSOutput main(PSInput input)
{
    PSOutput output;

    float4 outlineColor = pushConstants.Color;
    float target = pushConstants.SelectedEntity;
    float radius = pushConstants.Thickness;
	float2 uv = input.UV;

    int entityIDTextureID = pushConstants.EntityIDTextureID;
    float hasHit = 0.0f;

    float sampleValue = textures[entityIDTextureID].Sample(mySampler[0],, uv).r;
    float depth = textures[pushConstants.DepthTextureID].Sample(mySampler[0], uv).r;

    float4 fragColor = float4(0, 0, 0, 0);
    const float TAU = 6.28318530;
	const float steps = 32.0;
    for(float i = 0.0f; i < TAU; i += TAU / steps)
    {
        float2 uvOffset = float2(sin(i), cos(i)) * (GetTexelSize(textures[entityIDTextureID])) * radius;

        float2 sampleUV = uv + uvOffset;
        sampleUV.x = clamp(sampleUV.x, 0.0, 0.999);
		sampleUV.y = clamp(sampleUV.y, 0.0, 0.999);
        
        float sample = textures[entityIDTextureID].Sample(mySampler[0], sampleUV).r;
        float sampleDepth = textures[pushConstants.DepthTextureID].Sample(mySampler[0], sampleUV).r;
        if(sample == target && sampleDepth != 1.0f && sampleDepth > depth)
        {
            hasHit = 1.0f;
        }

        float alpha = smoothstep(0.5, 0.9, hasHit);
        float4 outputColor = float4(
			lerp(fragColor.r, outlineColor.r, alpha),
			lerp(fragColor.g, outlineColor.g, alpha),
			lerp(fragColor.b, outlineColor.b, alpha),
			lerp(fragColor.a, outlineColor.a, alpha)
		);

        fragColor = outputColor;
    }

    if(fragColor.a > 0.1)
    {
        fragColor.a = 1.0f;
    }

    float3 sourceTexture = textures[pushConstants.SourceTextureID].Sample(mySampler[0], uv).rgb;
    float ratio = float(sampleValue != target && hasHit > 0.0f);
    float4 finalColor = float4(
        lerp(sourceTexture.r, fragColor.r, ratio),
        lerp(sourceTexture.g, fragColor.g, ratio),
        lerp(sourceTexture.b, fragColor.b, ratio),
        lerp(1.0f, fragColor.a, ratio)
    );

    output.oColor0 = finalColor;
    return output;
}
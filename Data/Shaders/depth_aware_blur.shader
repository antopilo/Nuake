#shader vertex
#version 440 core

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 UVPosition;

out flat vec2 UV;

void main()
{
    UV = UVPosition;
    gl_Position = vec4(VertexPosition, 1.0f);
}

#shader fragment
#version 440 core
out vec4 FragColor;

in vec2 UV;

uniform sampler2D u_Input;
uniform sampler2D u_Depth;

const float SHARPNESS = 10.0f;
const float KERNEL_RADIUS = 8.0f;

void main() {

    vec2 texelSize = 1.0 / vec2(textureSize(u_Input, 0));
    vec4 result = vec4(0);

    float centerDepth = texture(u_Depth, UV).r;
    float totalWeight = 0.0f;
    for (int x = -2; x < 2; ++x)
    {
        vec2 offset = vec2(float(x), float(0)) * texelSize;

        vec2 uv = UV + offset;
        float depth = texture(u_Depth, uv).r;
           
        const float blurSigma = float(KERNEL_RADIUS) * 0.5f;
        const float blurFalloff = 1.0 / (2.0 * blurSigma * blurSigma);

        float depthDiff = abs(depth - centerDepth) * SHARPNESS;
        float weight = exp2(-x * x * blurFalloff - depthDiff * depthDiff);
        totalWeight += weight;
        result += texture(u_Input, uv) * weight;
    }

    for (int y = -2; y < 2; ++y)
    {
        vec2 offset = vec2(float(0), float(y)) * texelSize;

        vec2 uv = UV + offset;
        float depth = texture(u_Depth, uv).r;

        const float blurSigma = float(KERNEL_RADIUS) * 0.5f;
        const float blurFalloff = 1.0 / (2.0 * blurSigma * blurSigma);

        float depthDiff = abs(depth - centerDepth) * SHARPNESS;
        float weight = exp2(-y * y * blurFalloff - depthDiff * depthDiff);
        totalWeight += weight;
        result += texture(u_Input, uv) * weight;
    }

    FragColor = result / totalWeight;
}
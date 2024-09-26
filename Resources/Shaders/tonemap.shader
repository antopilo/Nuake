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

uniform sampler2D u_Source;
uniform float u_Gamma;
uniform float u_Exposure;
uniform float u_TAAFactor;
uniform sampler2D u_PreviousFrame;
uniform sampler2D u_VelocityFrame;

in vec2 UV;

out vec4 FragColor;

vec3 aces(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{
    vec3 color = texture(u_Source, UV).rgb;
    vec3 mapped = vec3(1.0) - exp(-color * u_Exposure);
    //color = vec3(1.0) - exp(-color * u_Exposure);
    // reinhard
    //color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(mapped, vec3(u_Gamma));

    // TAA
    vec2 velocity = texture(u_VelocityFrame, UV).rg;
    vec2 previousPixelPos = UV - velocity;

    vec3 historyColor = texture(u_PreviousFrame, previousPixelPos).rgb;

    // Sample the neighboring pixels for clamping
    vec3 NearColor0 = textureOffset(u_Source, UV, ivec2(1, 0)).rgb;
    vec3 NearColor1 = textureOffset(u_Source, UV, ivec2(0, 1)).rgb;
    vec3 NearColor2 = textureOffset(u_Source, UV, ivec2(-1, 0)).rgb;
    vec3 NearColor3 = textureOffset(u_Source, UV, ivec2(0, -1)).rgb;

    // Calculate the min and max colors
    vec3 BoxMin = min(color, min(NearColor0, min(NearColor1, min(NearColor2, NearColor3))));
    vec3 BoxMax = max(color, max(NearColor0, max(NearColor1, max(NearColor2, NearColor3))));

    // Clamp the history color
    historyColor = clamp(historyColor, BoxMin, BoxMax);

    color = mix(color, historyColor, u_TAAFactor);
    FragColor = vec4(color, 1.0);
}
#shader vertex
#version 460 core

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 UVPosition;

out flat vec2 UV;

void main()
{
    UV = UVPosition;
    gl_Position = vec4(VertexPosition, 1.0f);
}

#shader fragment
#version 460 core

uniform int u_Stage;

uniform sampler2D u_Source;
uniform vec2 u_SourceSize;
uniform float u_Exposure;
uniform float u_Gamma;
uniform sampler2D u_Source2;
uniform vec2 u_Source2Size;

uniform float u_Threshold;
uniform float u_BlurAmount;
uniform vec2 u_BlurDirection;

in vec2 UV;

out vec4 FragColor;

vec4 Threshold(vec4 color, vec2 uv)
{
    float brightness = max(color.r, max(color.g, color.b));
    float contribution = max(0, brightness - u_Threshold);
    contribution /= max(brightness, 0.00001);
    return color * contribution;
}

vec4 DownsampleBox13Tap(vec2 uv)
{
    vec2 texelSize = 1.0 / u_SourceSize;
    vec4 A = texture(u_Source, uv + texelSize * vec2(-1.0, -1.0));
    vec4 B = texture(u_Source, uv + texelSize * vec2(0.0, -1.0));
    vec4 C = texture(u_Source, uv + texelSize * vec2(1.0, -1.0));
    vec4 D = texture(u_Source, uv + texelSize * vec2(-0.5, -0.5));
    vec4 E = texture(u_Source, uv + texelSize * vec2(0.5, -0.5));
    vec4 F = texture(u_Source, uv + texelSize * vec2(-1.0, 0.0));
    vec4 G = texture(u_Source, uv);
    vec4 H = texture(u_Source, uv + texelSize * vec2(1.0, 0.0));
    vec4 I = texture(u_Source, uv + texelSize * vec2(-0.5, 0.5));
    vec4 J = texture(u_Source, uv + texelSize * vec2(0.5, 0.5));
    vec4 K = texture(u_Source, uv + texelSize * vec2(-1.0, 1.0));
    vec4 L = texture(u_Source, uv + texelSize * vec2(0.0, 1.0));
    vec4 M = texture(u_Source, uv + texelSize * vec2(1.0, 1.0));

    vec2 div = (1.0 / 4.0) * vec2(0.5, 0.125);

    vec4 o = (D + E + I + J) * div.x;
    o += (A + B + G + F) * div.y;
    o += (B + C + H + G) * div.y;
    o += (F + G + L + K) * div.y;
    o += (G + H + M + L) * div.y;

    return o;
}

vec4 Upsample(vec2 uv)
{
    vec4 d = (1.0 / u_Source2Size.xyxy) * vec4(1.0, 1.0, -1.0, 0.0) ;

    vec4 s;
    s = texture(u_Source2, uv - d.xy);
    s += texture(u_Source2, uv - d.wy) * 2.0;
    s += texture(u_Source2, uv - d.zy);

    s += texture(u_Source2, uv + d.zw) * 2.0;
    s += texture(u_Source2, uv) * 4.0;
    s += texture(u_Source2, uv + d.xw) * 2.0;

    s += texture(u_Source2, uv + d.zy);
    s += texture(u_Source2, uv + d.wy) * 2.0;
    s += texture(u_Source2, uv + d.xy);

    return s * (1.0 / 16.0);
}

vec4 blur9(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
    vec4 color = vec4(0.0);
    vec2 off1 = vec2(1.3846153846) * direction;
    vec2 off2 = vec2(3.2307692308) * direction;
    color += texture(image, uv) * 0.2270270270;
    color += texture(image, uv + (off1 / resolution)) * 0.3162162162;
    color += texture(image, uv - (off1 / resolution)) * 0.3162162162;
    color += texture(image, uv + (off2 / resolution)) * 0.0702702703;
    color += texture(image, uv - (off2 / resolution)) * 0.0702702703;
    return color;
}



void main()
{
    vec4 outputColor = texture(u_Source, UV).rgba;
    if (u_Stage == 0) // Threshold
    {
        outputColor = Threshold(outputColor, UV);
    }
    else if (u_Stage == 1) // Downsampling
    {
        outputColor = DownsampleBox13Tap(UV);
    }
    else if (u_Stage == 2) // Blur
    {
        outputColor = blur9(u_Source, UV, u_SourceSize, u_BlurDirection);
    }
    else if (u_Stage == 3) // Copy
    {
        outputColor = texture(u_Source, UV);
    }
    else if (u_Stage == 4) // Upsampling + combine
    {
        outputColor += Upsample(UV) / 2.0;
    }
    else if (u_Stage == 5) // Final combine
    {
        outputColor += texture(u_Source2, UV);

        vec3 color = outputColor.rgb;
        outputColor = vec4(color, 1.0);//vec4(acesOperator(outputColor.rgb), 1.0f);
    }
    FragColor = outputColor;
}
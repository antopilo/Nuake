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

    FragColor = vec4(color, 1.0);
}
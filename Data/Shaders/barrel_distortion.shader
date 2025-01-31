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

uniform sampler2D u_Source; // Input texture
uniform float u_Distortion;
uniform float u_DistortionEdge;
uniform float u_Scale;

in vec2 UV;

out vec4 FragColor;

// k1: the main distortion
// positive = barrel, negative = pincushion
// k2 : tweaks the edges of distortion
// can be 0.0
vec2 brownConradyDistortion(in vec2 uv, in float k1, in float k2)
{
    uv = uv * 2.0 - 1.0;	// brown conrady takes [-1:1]

    // positive values of K1 give barrel distortion, negative give pincushion
    float r2 = uv.x * uv.x + uv.y * uv.y;
    uv *= 1.0 + k1 * r2 + k2 * r2 * r2;

    // tangential distortion (due to off center lens elements)
    // is not modeled in this function, but if it was, the terms would go here
    uv *= u_Scale;
    uv = (uv * .5 + .5);	// restore -> [0:1]
    return uv;
}

void main()
{
    FragColor = texture(u_Source, brownConradyDistortion(UV, u_Distortion, u_DistortionEdge));
}
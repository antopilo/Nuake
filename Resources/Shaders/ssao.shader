#shader vertex
#version 440 core

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 UVPosition;

out flat vec2 UV;
out mat4 v_InvView;
out mat4 v_InvProjection;
out mat4 v_View;
out mat4 v_Projection;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    v_View = u_View;
    v_Projection = u_Projection;
    v_InvView = inverse(u_View);
    v_InvProjection = inverse(u_Projection);

	UV = UVPosition;
    gl_Position = vec4(VertexPosition, 1.0f);
}

#shader fragment
#version 440 core

uniform sampler2D u_Depth;
uniform sampler2D u_Normal;
uniform sampler2D u_Noise;
uniform int u_KernelSize = 64;

uniform vec3 u_Samples[64];
uniform mat4 u_Projection;
uniform vec2 u_NoiseScale;
uniform float u_Radius = 0.5f;
uniform float u_Bias = 0.025f;
uniform float u_Falloff = 0.0022;
uniform float u_Area = 0.0075;
uniform float u_Strength = 2.0f;

in vec2 UV;
in mat4 v_View;
in mat4 v_InvView;
in mat4 v_InvProjection;
in mat4 v_Projection;

out vec4 FragColor;

float linearize_depth(float d, float zNear, float zFar)
{
    return zNear * zFar / (zFar + d * (zNear - zFar));
}

vec3 WorldPosFromDepth(float depth) 
{
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(UV * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = v_InvProjection * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = v_InvView * viewSpacePosition;

    return worldSpacePosition.xyz;
}

vec3 ViewPosFromDepth(float depth)
{
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(UV * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = v_InvProjection * clipSpacePosition;
    viewSpacePosition.xyz /= viewSpacePosition.w;

    return viewSpacePosition.xyz;
}

vec3 normal_from_depth(float depth, vec2 texcoords) {
  
  const vec2 offset1 = vec2(0.0,0.0002);
  const vec2 offset2 = vec2(0.0002,0.0);
  
  float depth1 = texture(u_Depth, UV + offset1).r;
  float depth2 = texture(u_Depth, UV + offset2).r;
  
  vec3 p1 = vec3(offset1, depth1 - depth);
  vec3 p2 = vec3(offset2, depth2 - depth);
  
  vec3 normal = cross(p1, p2);
  normal.z = -normal.z;
  
  return normalize(normal);
}

void main()
{
    float depth = texture(u_Depth, UV).r;
    if (depth > 0.9999999f)
    {
        FragColor = vec4(0, 0, 0, 0);
        return;
    }

    const float SCALING_NEAR = 0.92;
    float depthScaler = (depth - SCALING_NEAR) / (1.0 - SCALING_NEAR);

    const float minRadius = 0.05f;
    const float maxRadius = 1.2f;
    const float scalerPow = 1.8f;
    depthScaler = min(max(pow(depthScaler, scalerPow), minRadius), maxRadius);
    float scaledRadius = u_Radius;

    vec3 fragPos = ViewPosFromDepth(depth);
    vec3 normal = texture(u_Normal, UV).xyz * 2.0 - 1.0;//normal_from_depth(depth, UV);
    normal.z *= -1.0f;
    // Remove translation from view;
    mat4 invView = v_InvView;
    invView[3] = vec4(0, 0, 0, 1);

    normal = (invView * vec4(normal, 1.0f)).xyz;
    vec3 randomVec = texture(u_Noise, UV * u_NoiseScale).xyz;

    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for (int i = 0; i < 64; i++)
    {
        vec3 samplePos = TBN * u_Samples[i]; // generate a random point
        samplePos.z *= -1.0f;
        samplePos = fragPos + samplePos * scaledRadius;

        vec4 offset = vec4(samplePos, 1.0); // make it a 4-vector
        offset = v_Projection * offset; // project on the near clipping plane
        offset.xyz /= offset.w; // perform perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to (0,1) range
        float sampleDepth = texture(u_Depth, offset.xy).r;
        float rangeCheck = smoothstep(0.0, 1.0, scaledRadius / abs((samplePos.z) - sampleDepth));
        occlusion += (sampleDepth <= depth - u_Bias / 100.0 ? 1.0 : 0.0) * rangeCheck;
    }

    float ao = 1.0 - (occlusion / 64.0);

    float finalAO = pow(ao, u_Strength * 100.0);
    FragColor = vec4(finalAO, finalAO, finalAO, 1.0);
}
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

uniform sampler2D u_Normal;
uniform sampler2D u_Depth;
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
    return (2.0 * zNear * zFar) / (zFar + zNear - d * (zFar - zNear)) / 400.0;
}

vec3 WorldPosFromDepth(float depth) 
{
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(UV * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = v_InvProjection * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    return viewSpacePosition.xyz;
}


void main()
{
    float depth = texture(u_Depth, UV).r;
    if (depth > 0.9999999f)
    {
        FragColor = vec4(1, 1, 1, 0);
        return;
    }

    vec3 fragPos = WorldPosFromDepth(depth);
    vec3 worldSpaceNormal = texture(u_Normal, UV).xyz * 2.0 - 1.0;
    mat3 normalMatrix = transpose(inverse(mat3(v_View)));
    vec3 normal = (normalMatrix * worldSpaceNormal).xyz;

    vec3 randomVec = texture(u_Noise, UV * u_NoiseScale).xyz;

    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0f;
    
    const int kernelCount = 64;
    for (int i = 0; i < kernelCount; i++)
    {
        vec3 samplePos = TBN * u_Samples[i]; // generate a random point
        samplePos = fragPos + samplePos * u_Radius;

        vec4 offset = vec4(samplePos, 1.0); // make it a 4-vector
        offset = v_Projection * offset; // project on the near clipping plane
        offset.xyz /= offset.w; // perform perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to (0,1) range

        offset.x = clamp(offset.x, 0.00001, 0.999);
        offset.y = clamp(offset.y, 0.00001, 0.999);

        float sampleDepth = texture(u_Depth, offset.xy).x;
        float epsilon = 0.000001f; // Adjust based on your scene's scale
        float realBias = (-0.001 / 1000.0f);
        float rangeCheck = smoothstep(0.0, 1.0, (u_Radius / 500000.0f) / (abs(depth - sampleDepth))) ;
        occlusion += (sampleDepth <= offset.z + realBias ? 1.0f : 0.0f) * rangeCheck ;
    }
    
    float ao = 1.0 - pow(occlusion, u_Strength) / 64.0f;
    FragColor = vec4(ao, ao, ao, 1.0f);
}
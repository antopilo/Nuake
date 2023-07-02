#shader vertex
#version 460 core

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 UVPosition;

out flat vec2 UV;
out mat4 v_InvView;
out mat4 v_InvProjection;
out mat4 v_View;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    v_View = u_View;
    v_InvView = inverse(u_View);
    v_InvProjection = inverse(u_Projection);

	UV = UVPosition;
    gl_Position = vec4(VertexPosition, 1.0f);
}

#shader fragment
#version 460 core

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

out vec4 FragColor;

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
    float depth = texture(u_Depth, UV).r ;
    vec3 fragPos = ViewPosFromDepth(depth);
    float normalizedRadius = u_Radius;
    if(depth > 0.9999999f)
    {
        FragColor = vec4(0, 0, 0, 0);
        return;
    }
    vec3 goodNormal = normal_from_depth(depth, UV);
    vec3 normal = texture(u_Normal, UV).rgb * 2.0 - 1.0;
    vec4 normalT = v_View * vec4(normal, 1.0);
    normalT *= -1.0f;
    normal = normalT.xyz;
    normal = normalize(normal);
    normal = goodNormal;
    vec3 randomVec = texture(u_Noise, UV * u_NoiseScale).xyz;  
    float radius_depth = normalizedRadius / depth;
    float occlusion = 0.0;
    vec3 position = vec3(UV, depth);
    
    int skipped = 0;
    float rangeCheckC = 0.0f;
    for(int i=0; i < 64; i++) 
    {
        vec3 ray = u_Radius * reflect(u_Samples[i], randomVec);
        vec3 hemi_ray = position + sign(dot(ray, normal)) * ray;
        
        float sampleDepth = texture(u_Depth, hemi_ray.xy).r;
        float rangeCheck = smoothstep(0.0, 1.0, u_Radius * 0.8 / abs(depth - sampleDepth));
        float ao = hemi_ray.z >= sampleDepth + u_Bias ? 1.0 : 0.0;
        occlusion += ao * rangeCheck;
        rangeCheckC += rangeCheck;
    }
  
    float ao = 1.0 - (occlusion / 64.0);
    rangeCheckC /= 64.0;

    float finalAO = pow(ao, u_Strength);
    FragColor = vec4(finalAO, finalAO, finalAO, 1.0);
    //FragColor = vec4(rangeCheckC, rangeCheckC, rangeCheckC, 1.0);

    //FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
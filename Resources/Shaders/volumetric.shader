#shader vertex
#version 440 core

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 UVPosition;

out flat vec2 UV;
out mat4 InvView;
out mat4 InvProjection;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    UV = UVPosition;
    InvView = inverse(u_View);
    InvProjection = inverse(u_Projection);
    gl_Position = vec4(VertexPosition, 1.0f);
}

#shader fragment
#version 440 core

float ditherPattern[4][4] = { { 0.0f, 0.5f, 0.125f, 0.625f},
{ 0.75f, 0.22f, 0.875f, 0.375f},
{ 0.1875f, 0.6875f, 0.0625f, 0.5625},
{ 0.9375f, 0.4375f, 0.8125f, 0.3125} };

in mat4 InvView;
in mat4 InvProjection;

uniform sampler2D u_Depth;

uniform vec3 u_CamPosition;
uniform int u_StepCount;
uniform float u_FogAmount;

const int MAX_LIGHT = 20;
uniform int u_LightCount;
struct Light {
    mat4 transform;
    vec3 color;
    vec3 direction;
    sampler2D shadowmap;
    float strength;
};

uniform Light u_Lights[MAX_LIGHT];

uniform sampler2D lightShadowmap;
in vec2 UV;

out vec4 FragColor;
const float PI = 3.141592653589793f;

// Mie scaterring approximated with Henyey-Greenstein phase function.
float ComputeScattering(float lightDotView)
{
    float result = 1.0f - u_FogAmount ;
    result /= (4.0f * PI * pow(1.0f + u_FogAmount * u_FogAmount - (2.0f * u_FogAmount) * lightDotView, 1.5f));
    return result;
}

vec3 ComputeVolumetric(vec3 FragPos, Light light)
{
    vec3 startPosition = u_CamPosition;             // Camera Position
    vec3 rayVector = FragPos - startPosition;  // Ray Direction

    float rayLength = length(rayVector);            // Length of the raymarched
    //if(rayLength > 1000.0)
    //    return vec3(1.0, 1.0, 1.0);
    float stepLength = rayLength / u_StepCount;        // Step length
    vec3 rayDirection = rayVector / rayLength;
    vec3 step = rayDirection * stepLength;          // Normalized to step length direction

    vec3 accumFog = vec3(0.0f, 0.0f, 0.0f);         // accumulative color

    // Raymarching
    vec3 currentPosition = startPosition;
    for (int i = 0; i < u_StepCount; i++)
    {
        vec4 fragPosLightSpace = light.transform * vec4(currentPosition, 1.0f);
        // perform perspective divide
        vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

        // transform to [0,1] range
        projCoords = projCoords * 0.5 + 0.5;

        float currentDepth = projCoords.z;
        float closestDepth = texture(light.shadowmap, projCoords.xy).r;
        if (closestDepth > currentDepth && closestDepth < 999)
        {
            //accumFog = vec3(light.color);
            accumFog += (ComputeScattering(dot(rayDirection, light.direction)).xxx * light.color );
            //accumFog = vec3(projCoords.x, projCoords.y, 1.0);
            
        }
        currentPosition += step * ditherPattern[int(gl_FragCoord.x) % 4][int(gl_FragCoord.y) % 4];
        //accumFog = vec3(projCoords);
    }
    accumFog /= u_StepCount;

    return accumFog * 4.0;
}

// Converts depth to World space coords.
vec3 WorldPosFromDepth(float depth) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(UV * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = InvProjection * clipSpacePosition;

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = InvView * viewSpacePosition;

    return worldSpacePosition.xyz;
}

void main()
{
    float depth = texture(u_Depth, UV).r;
    vec3 globalFragmentPosition = WorldPosFromDepth(depth);

    vec3 fog = vec3(0, 0, 0);
    for (int i = 0; i < u_LightCount; i++)
    {
        fog += ComputeVolumetric(globalFragmentPosition, u_Lights[i]);
    }

    FragColor = vec4(fog, 1.0);
    //FragColor = vec4(mix(fog, ComputeVolumetric(globalFragmentPosition, u_Lights[0]), 0.9f), 0.01);
    //FragColor = vec4(globalFragmentPosition * 10.0, 1.0f);
    //FragColor = vec4(globalFragmentPosition.xyz * 100.0, 1.0f);
    //FragColor = vec4(worldSpacePosition.x, worldSpacePosition.y, worldSpacePosition.z, 1);
    //FragColor = vec4(ComputeVolumetric(globalFragmentPosition, u_Lights[0]), 1.0);
}
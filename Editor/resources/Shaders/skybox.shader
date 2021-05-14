#shader vertex
#version 460 core

layout(location = 0) in vec3 vertexPosition;

out vec3 TexCoords;
out vec3 LocalPos;

uniform mat4 projection;
uniform mat4 view;
void main()
{
    TexCoords = vertexPosition;
    LocalPos = vertexPosition;
    gl_Position = projection * mat4(mat3(view)) * vec4(vertexPosition, 1.0);
}

#shader fragment
#version 460 core
out vec4 FragColor;

in vec3 TexCoords;
in vec3 LocalPos;

uniform int isProcedural;
uniform int isHDR;
uniform int convulate;
uniform sampler2D equirectangularMap;
uniform samplerCube skybox;
uniform float roughness;
uniform int prefilter;

const float PI = 3.14159265359;
const vec2 invAtan = vec2(0.1591, 0.3183);


vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness * roughness;

    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // from tangent-space vector to world-space sample vector
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

void main()
{
    if (prefilter == 1) {
        vec3 N = normalize(LocalPos);
        vec3 R = N;
        vec3 V = R;

        const uint SAMPLE_COUNT = 2048;
        float totalWeight = 0.0;
        vec3 prefilteredColor = vec3(0.0);
        for (uint i = 0u; i < SAMPLE_COUNT; ++i)
        {
            vec2 Xi = Hammersley(i, SAMPLE_COUNT);
            vec3 H = ImportanceSampleGGX(Xi, N, roughness);
            vec3 L = normalize(2.0 * dot(V, H) * H - V);


            float NdotL = max(dot(N, L), 0.0);
            if (NdotL > 0.0)
            {
                // sample from the environment's mip level based on roughness/pdf
                float D = DistributionGGX(N, H, roughness);
                float NdotH = max(dot(N, H), 0.0);
                float HdotV = max(dot(H, V), 0.0);
                float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

                float resolution = 1024.0; // resolution of source cubemap (per face)
                float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
                float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

                float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

                prefilteredColor += textureLod(skybox, L, mipLevel).rgb * NdotL;
                totalWeight += NdotL;
            }
        }
        prefilteredColor = prefilteredColor / totalWeight;

        FragColor = vec4(prefilteredColor, 1.0);
    }
    else if (convulate == 1) {
        // The world vector acts as the normal of a tangent surface
        // from the origin, aligned to WorldPos. Given this normal, calculate all
        // incoming radiance of the environment. The result of this radiance
        // is the radiance of light coming from -Normal direction, which is what
        // we use in the PBR shader to sample irradiance.
        vec3 N = normalize(LocalPos);

        vec3 irradiance = vec3(0.0);

        // tangent space calculation from origin point
        vec3 up = vec3(0.0, 1.0, 0.0);
        vec3 right = cross(up, N);
        up = cross(N, right);

        float sampleDelta = 0.025;
        float nrSamples = 0.0;
        for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
        {
            for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
            {
                // spherical to cartesian (in tangent space)
                vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
                // tangent space to world
                vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

                irradiance += texture(skybox, sampleVec).rgb * cos(theta) * sin(theta);
                nrSamples++;
            }
        }
        irradiance = PI * irradiance * (1.0 / float(nrSamples));

        FragColor = vec4(irradiance, 1.0);
    }
    else {
        if (isHDR == 1) {
            vec2 uv = SampleSphericalMap(normalize(LocalPos)); // make sure to normalize localPos
            vec3 color = texture(equirectangularMap, uv).rgb;
            FragColor = vec4(color, 1.0);
        }
        else {
            vec4 sky = textureLod(skybox, LocalPos, 1.2); //texture(skybox, TexCoords);
            FragColor = sky;
        }
    }
}

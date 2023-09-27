#shader vertex
#version 460 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 UVPosition;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 Tangent;
layout(location = 4) in vec3 Bitangent;
layout(location = 5) in ivec4 BoneIDs;
layout(location = 6) in vec4 Weights;

uniform mat4 u_LightTransform;

const int MAX_BONES = 200;
const int MAX_BONES_INFLUENCE = 4;
uniform mat4 u_FinalBonesMatrice[MAX_BONES];

void main()
{
    vec4 totalPosition = vec4(0.0f);
    for (int i = 0; i < MAX_BONES_INFLUENCE; i++)
    {
        if (BoneIDs[i] == -1)
        {
            continue;
        }

        if (BoneIDs[i] >= MAX_BONES)
        {
            totalPosition = vec4(Position, 1.0f);
            break;
        }

        vec4 localPosition = u_FinalBonesMatrice[BoneIDs[i]] * vec4(Position, 1.0f);
        totalPosition += localPosition * Weights[i];
        // vec3 localNormal = mat3(u_FinalBonesMatrice[BoneIDs[i]]) * Normal;
    }

    gl_Position = u_LightTransform * totalPosition;
}

#shader fragment
#version 460 core

void main() { }
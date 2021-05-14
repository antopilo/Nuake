#shader vertex
#version 460 core
layout(location = 0) in vec3 Position;
layout(location = 2) in vec3 Normal;
uniform mat4 lightSpaceMatrix;
uniform mat4 model;

out vec4 WorldNormal;
out vec4 Pos;
void main()
{
    Pos = model * vec4(Position, 1.0);
    WorldNormal = model * vec4(Normal, 1.0);
    gl_Position = lightSpaceMatrix * model * vec4(Position, 1.0);
}

#shader fragment
#version 460 core

in vec4 WorldNormal;
in vec4 Pos;

out vec4 Color;
layout(location = 0) out vec4 Flux;
layout(location = 1) out vec4 Normal;
layout(location = 2) out vec4 Position;
void main()
{
    Normal = vec4(WorldNormal.x, WorldNormal.y, WorldNormal.z, 1.0);
    Flux = vec4(0.0, 1.0, 0.0, 1.0);
    Position = Pos;
    Position.a = 1.0f;
}

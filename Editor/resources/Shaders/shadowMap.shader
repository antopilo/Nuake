#shader vertex
#version 460 core

layout(location = 0) in vec3 Position;

uniform mat4 u_LightTransform;
uniform mat4 u_Model;

void main()
{
    gl_Position = u_LightTransform * u_Model * vec4(Position, 1.0);
}

#shader fragment
#version 460 core

void main() { }
#shader vertex
#version 460 core
layout(location = 0) in vec3 Position;
uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{

    gl_Position = lightSpaceMatrix * model * vec4(Position, 1.0);
}

#shader fragment
#version 460 core


void main()
{

}

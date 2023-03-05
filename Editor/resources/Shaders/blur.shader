#shader vertex
#version 460 core

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 UVPosition;

out flat vec2 UV;

void main()
{
	UV = UVPosition;
    gl_Position = vec4(VertexPosition, 1.0f);
}

#shader fragment
#version 460 core
out vec4 FragColor;
  
in vec2 UV;
  
uniform sampler2D u_Input;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(u_Input, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(u_Input, UV + offset).r;
        }
    }
    float channel = result / (4.0 * 4.0);
    FragColor = vec4(channel, channel, channel, 1.0);
} 
#shader vertex
#version 440 core

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 UVPosition;

out flat vec2 UV;

void main()
{
    UV = UVPosition;
    gl_Position = vec4(VertexPosition, 1.0f);
}

#shader fragment
#version 440 core

uniform int u_PixelSize = 4;

uniform sampler2D u_Source;
uniform vec2 u_SourceSize;

in vec2 UV;

out vec4 FragColor;

// Implementation from: https://lettier.github.io/3d-game-shaders-for-beginners/pixelization.html
void main()
{
    vec2 pixelCoord = UV * u_SourceSize;
    float x = int(pixelCoord.x) % u_PixelSize;
    float y = int(pixelCoord.y) % u_PixelSize;

    x = floor(u_PixelSize / 2.0) - x;
    y = floor(u_PixelSize / 2.0) - y;

    x = pixelCoord.x + x;
    y = pixelCoord.y + y;
    FragColor = texture(u_Source, vec2(x, y) / u_SourceSize);
}
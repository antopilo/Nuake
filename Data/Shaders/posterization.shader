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

uniform sampler2D u_Source;
uniform int u_Levels = 10;
in vec2 UV;

out vec4 FragColor;

// Implementation from: https://lettier.github.io/3d-game-shaders-for-beginners/posterization.html
void main()
{
    vec4 frameColor = texture(u_Source, UV);
    float greyscale = max(frameColor.r, max(frameColor.g, frameColor.b));
    float lower = floor(greyscale * u_Levels) / u_Levels;
    float lowerDiff = abs(greyscale - lower);
    float upper     = ceil(greyscale * u_Levels) / u_Levels;
    float upperDiff = abs(upper - greyscale);
    float level      = lowerDiff <= upperDiff ? lower : upper;
    float adjustment = level / greyscale;
    FragColor = frameColor * adjustment;
}
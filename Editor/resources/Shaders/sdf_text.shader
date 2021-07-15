#shader vertex
#version 460 core
layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 UV;

uniform mat4 model;
uniform mat4 projection;

out sample vec2 a_UV;

void main()
{
    a_UV = UV;
    gl_Position = projection * model * vec4(Position, 1.0);
}

#shader fragment
#version 460 core
out vec4 FragColor;

in sample vec2 a_UV;

uniform vec2 texPos;
uniform vec2 texScale;
uniform sampler2D msdf;
uniform vec4 bgColor;
uniform vec4 fgColor;

uniform float pxRange; // set to distance field's pixel range

float screenPxRange(vec2 coord) {
    vec2 unitRange = vec2(pxRange) / vec2(textureSize(msdf, 0));
    vec2 screenTexSize = vec2(1.0) / fwidth(coord);
    return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {
    vec2 textSize = textureSize(msdf, 0);

    vec2 uv = vec2(mix(texPos.x / textSize.x, texScale.x / textSize.x, a_UV.x),
                   mix(texScale.y / textSize.y, texPos.y / textSize.y, a_UV.y));

    vec3 msd = texture(msdf, uv).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxRange(uv) * (sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);

    FragColor = mix(bgColor, fgColor, opacity);
}
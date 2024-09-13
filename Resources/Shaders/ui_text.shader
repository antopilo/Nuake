#shader vertex
#version 460 core
layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 UV;

uniform mat4 u_Model;
uniform mat4 u_View;

out vec2 v_UV;

void main() {
	v_UV = UV;
	float x = Position.x;
	float y = Position.y;
	float z = Position.z;

	gl_Position = u_View * u_Model * vec4(x, y, z, 1.0f);
}

#shader fragment
#version 460 core

in sample vec2 v_UV;
out vec4 FragColor;

uniform sampler2D u_Atlas;
uniform sample vec2 u_TexturePos;
uniform vec2 u_TextureScale;
uniform vec4 u_FontColor;
uniform float u_PxRange;

uniform float u_SubpixelThreshold;
uniform float u_CurveTolerance;
uniform float u_SubpixelAmount;

float screenPxRange(vec2 coord) {
    vec2 unitRange = vec2(u_PxRange) / vec2(textureSize(u_Atlas, 0));
    vec2 screenTexSize = vec2(1.0) / fwidth(coord);
    return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {
    vec2 textSize = textureSize(u_Atlas, 0);

    vec2 uv = vec2(mix(u_TexturePos.x / textSize.x, u_TextureScale.x / textSize.x, v_UV.x),
                   mix(u_TextureScale.y / textSize.y, u_TexturePos.y / textSize.y, 1.0 - v_UV.y));

    vec3 msd = texture(u_Atlas, uv).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxRange(uv) * (sd - 0.5);
    vec3 color = u_FontColor.rgb;
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    FragColor = vec4(color.rgb, opacity);
}
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

uniform sampler2D u_Source;

uniform float u_Intensity;
uniform float u_Extend;

in vec2 UV;

out vec4 FragColor;

void main()
{
    vec2 uv = UV;

    uv *= 1.0 - uv.yx;   //vec2(1.0)- uv.yx; -> 1.-u.yx; Thanks FabriceNeyret !

    float vig = uv.x * uv.y * u_Intensity; // multiply with sth for intensity

    vig = pow(vig, u_Extend); // change pow for modifying the extend of the  vignette
    vig = clamp(vig, 0, 1);
    FragColor = vec4(texture(u_Source, UV).rgb * vig, 1.0) ;

}
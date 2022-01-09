#shader vertex
#version 460 core

layout(location = 0) in vec3 VertexPosition;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;

void main()
{
    gl_Position = u_Projection * u_View * u_Model * vec4(VertexPosition, 1.0f);
}

#shader fragment
#version 460 core

uniform vec4 u_Color;
uniform float u_Time;
out vec4 FragColor;

void main()
{
    if (mod(gl_FragCoord.x + gl_FragCoord.y + (u_Time * 4.0), 4.0) < 2.0)
        discard;

    vec4 finalColor = u_Color;
    finalColor.a = (sin(u_Time * 4.0) + 1.0) / 4.0 + 0.1;
    FragColor = finalColor;
    
}
#shader vertex
#version 460 core


layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 Normal;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;

out vec3 v_Normal;

void main()
{
    v_Normal = Normal;
    gl_Position = u_Projection * u_View * u_Model * vec4(VertexPosition, 1.0f);
}

#shader fragment
#version 460 core

uniform vec4 u_Color;

out vec4 FragColor;
in vec3 v_Normal;

void main()
{
    vec3 scolor = u_Color.rgb;
    vec3 lightAmbient = vec3(0.5, 0.5, 0.5);
    vec3 lightDiffuse = vec3(1, 1, 1);
    vec3 lightDir = normalize(vec3(0.5, 0.5, 0.5));
    lightDir = normalize(lightDir);
    // diffuse shading
    float diff = max(dot(v_Normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, v_Normal);
    // combine results
    vec3 ambient  = lightAmbient * scolor;
    vec3 diffuse  = diff * lightDiffuse;
    scolor = scolor * diffuse;

    FragColor = vec4(scolor, 1.0);
}
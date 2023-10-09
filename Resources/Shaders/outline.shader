#shader vertex
#version 440 core

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 UVPosition;

out flat vec2 a_UV;

void main()
{
	a_UV = UVPosition;
	gl_Position = vec4(VertexPosition, 1.0f);
}

#shader fragment
#version 440 core

uniform int u_EntityID;
uniform usampler2D u_EntityTexture;
uniform vec4 u_OutlineColor;

out vec4 FragColor;

in vec2 a_UV;

void main()
{
    int target = u_EntityID;
    const float TAU = 6.28318530;
	const float steps = 32.0;
    
	float radius = 4.f;
	vec2 uv = a_UV;
    
    // Correct aspect ratio
    vec2 aspect = 1.0 / vec2(textureSize(u_EntityTexture, 0));
    
	vec4 fragColor = vec4(0.0, 0.0, 0.0, 0.0f);
	for (float i = 0.0; i < TAU; i += TAU / steps) 
    {
		// Sample image in a circular pattern
        vec2 offset = vec2(sin(i), cos(i)) * aspect * radius;
		uint col = texture(u_EntityTexture, uv + offset).r;
		
		// Mix outline with background
		float alpha = smoothstep(0.5, 0.7, int(col != target) * 10.0f);
		fragColor = mix(fragColor, u_OutlineColor, alpha);
	}
	
    if(fragColor.a > 0.1)
    {
        fragColor.a = 1.0f;
    }
    
    FragColor = mix(vec4(0), fragColor, texture(u_EntityTexture, uv).r == target);
}

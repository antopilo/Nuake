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
uniform sampler2D u_Depth;

out vec4 FragColor;

in vec2 a_UV;

void main()
{
    int target = u_EntityID;
    const float TAU = 6.28318530;
	const float steps = 32.0;
    
	float radius = 4.f;
	vec2 uv = a_UV;
    
	// sample middle
	uint middleSample = texture(u_EntityTexture, uv).r;
	float depth = texture(u_Depth, uv).r;
	
    // Correct aspect ratio
    vec2 aspect = 1.0 / vec2(textureSize(u_EntityTexture, 0));
    
	float hasHit = 0.0f;
	vec4 fragColor = vec4(0.0, 0.0, 0.0, 0.0f);
	for (float i = 0.0; i < TAU; i += TAU / steps) 
    {
		// Sample image in a circular pattern
        vec2 offset = vec2(sin(i), cos(i)) * aspect * radius;
		
		// We dont want to sample outside the viewport
		vec2 sampleUv = uv + offset;
		sampleUv.x = clamp(sampleUv.x, 0.0, 0.999);
		sampleUv.y = clamp(sampleUv.y, 0.0, 0.999);

		uint col = texture(u_EntityTexture, sampleUv).r;
		float depthTarget = texture(u_Depth, sampleUv).r;
		if(col == target && depthTarget != 0.0f && depthTarget < depth)
		{
			hasHit = 1.0f;
		}

		// Mix outline with background
		float alpha = smoothstep(0.5, 0.9, int(col != target) * hasHit * 10.0f);
		fragColor = mix(fragColor, u_OutlineColor, alpha);
	}
	
    if(fragColor.a > 0.1)
    {
        fragColor.a = 1.0f;
    }
    
    FragColor = mix(vec4(0), fragColor, middleSample != target && hasHit > 0.0f);
}

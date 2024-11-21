#shader vertex
#version 460 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 UV;

uniform mat4 u_Model;
uniform mat4 u_View;

out vec2 v_UV;

void main() {
	v_UV = UV;
	gl_Position = u_View * u_Model * vec4(Position, 1.0f);
}

#shader fragment
#version 460 core

in vec2 v_UV;
out vec4 FragColor;

uniform vec2 u_Size;
uniform vec4 u_Color;
uniform vec4 u_BorderColor;
uniform float u_Border;
uniform float u_BorderRadius;
uniform sampler2D u_BackgroundImage;
uniform float u_HasBackgroundImage;

float ShouldDiscard(vec2 coords, float border, vec2 dimensions, float radius)
{
    vec2 circle_center = vec2(radius + border, radius + border) - vec2(0.5, 0.5);
    float dst = length(coords - circle_center);
    float delta = fwidth(dst);
    if (coords.x < circle_center.x && coords.y < circle_center.y) 
        return 1.0 - smoothstep(radius - delta, radius, dst);

    circle_center.x += dimensions.x - 2 * (radius + border) + 1.f;
    dst = length(coords - circle_center);
    delta = fwidth(dst);
    if (coords.x > circle_center.x && coords.y < circle_center.y) 
        return 1.0 - smoothstep(radius - delta, radius, dst);

    circle_center.y += dimensions.y - 2 * (radius + border) + 1.f;
    dst = length(coords - circle_center);
    delta = fwidth(dst);
    if (coords.x > circle_center.x && coords.y > circle_center.y) 
        return 1.0 - smoothstep(radius - delta, radius, dst); 

    circle_center.x -= dimensions.x - 2 * (radius + border) + 1.f;
    dst = length(coords - circle_center);
    delta = fwidth(dst);
    if (coords.x < circle_center.x && coords.y > circle_center.y) 
        return 1.0 - smoothstep(radius - delta, radius, dst);

    return 1.f;

}

void main() { 
	vec2 coords = v_UV * u_Size;
	vec2 center = u_Size / 2.0;
	vec4 finalColor = u_Color;

    vec4 textureColor = texture(u_BackgroundImage, v_UV);
	finalColor.rgb = mix(finalColor, textureColor, u_HasBackgroundImage).rgb;

	if (u_HasBackgroundImage == 1.0)
	{
		finalColor.a = textureColor.a;
	}

    float a = 1.0f;
    if(u_BorderRadius >= 0.f)
    {        
        a = min(ShouldDiscard(coords, 0, u_Size, u_BorderRadius + 2), finalColor.a);
        if(a == 0.f) 
            discard;
    }
    
    if(u_Border > 0.f)
    {
        float borderAlpha = ShouldDiscard(coords, u_Border, u_Size, u_BorderRadius);


		if(a == 1.f)
        { 
			finalColor.rgb = mix(u_BorderColor.rgb, finalColor.rgb, borderAlpha);
            if(coords.x < u_Border || coords.x > u_Size.x - u_Border ||
                coords.y < u_Border || coords.y > u_Size.y - u_Border)
            {
                 finalColor = u_BorderColor;
				
            }
			else
			{
				finalColor.a = mix(borderAlpha, textureColor.a, u_HasBackgroundImage);
                
            }
			
            FragColor = finalColor; 
            return;
        }
        else
        {
            finalColor = u_BorderColor;
            finalColor.a = max(a, u_BorderColor.a);
            FragColor = finalColor;
            return;
        }
    }
    
	FragColor = finalColor; 
}
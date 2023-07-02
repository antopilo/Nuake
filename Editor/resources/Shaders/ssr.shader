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
#version 420 core

in vec2 UV;
out vec4 outColor;

uniform sampler2D textureFrame;
uniform sampler2D textureDepth;
uniform sampler2D textureNorm;
uniform sampler2D textureMetallic;
uniform sampler2D textureAlbedo;

uniform mat4 proj;
uniform mat4 invProj;
uniform mat4 view;
uniform mat4 invView;

uniform float rayStep = 0.2f;
uniform int iterationCount = 100;
uniform float distanceBias = 0.05f;
uniform bool enableSSR = true;
uniform int sampleCount = 4;
uniform bool isSamplingEnabled = true;
uniform bool isExponentialStepEnabled = false;
uniform bool isAdaptiveStepEnabled = true;
uniform bool isBinarySearchEnabled = true;
uniform bool debugDraw = false;
uniform float samplingCoefficient = 0.001f;

vec3 ReflectedVector;
float Metallic;
const float reflectionSpecularFalloffExponent = 3.0;
float random(vec2 uv) {
	return fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453123); //simple random function
}

vec3 generatePositionFromDepth(vec2 texturePos, float depth) {
	vec4 ndc = vec4((texturePos - 0.5) * 2, depth, 1.f);
	vec4 inversed = invProj * ndc;// going back from projected
	inversed /= inversed.w;
	return inversed.xyz;
}

vec2 generateProjectedPosition(vec3 pos) {
	vec4 samplePosition = proj * vec4(pos, 1.f);
	samplePosition.xy = (samplePosition.xy / samplePosition.w) * 0.5 + 0.5;
	return samplePosition.xy;
}

vec3 SSR(vec3 position, vec3 reflection) {
	vec3 step = rayStep * reflection;
	vec3 marchingPosition = position + step;
	float delta;
	float depthFromScreen;
	vec2 screenPosition;

	int i = 0;
	for (; i < iterationCount; i++) {
		screenPosition = generateProjectedPosition(marchingPosition);
		depthFromScreen = abs(generatePositionFromDepth(screenPosition, textureLod(textureDepth, screenPosition, 2).x).z);
		delta = abs(marchingPosition.z) - depthFromScreen;

		float depth = textureLod(textureDepth, screenPosition, 2).r;

		if (abs(delta) < distanceBias || depthFromScreen > 1000.0f) {
			vec3 color = vec3(1);
			if (debugDraw)
				color = vec3(0.5 + sign(delta) / 2, 0.3, 0.5 - sign(delta) / 2);
			vec2 dCoords = smoothstep(0.2, 0.6, abs(vec2(0.5, 0.5) - screenPosition.xy));
			float screenEdgefactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);
			float ReflectionMultiplier = pow(Metallic, 3.0) *
				screenEdgefactor *
				-ReflectedVector.z;
			
			return texture(textureFrame, screenPosition).xyz * color * clamp(ReflectionMultiplier, 0.0, 0.9);
		}
		if (isBinarySearchEnabled && delta > 0) {
			break;
		}
		if (isAdaptiveStepEnabled) {
			float directionSign = sign(abs(marchingPosition.z) - depthFromScreen);
			//this is sort of adapting step, should prevent lining reflection by doing sort of iterative converging
			//some implementation doing it by binary search, but I found this idea more cheaty and way easier to implement
			step = step * (1.0 - rayStep * max(directionSign, 0.0));
			marchingPosition += step * (-directionSign);
		}
		else {
			marchingPosition += step;
		}
		if (isExponentialStepEnabled) {
			step *= 1.05;
		}
	}
	if (isBinarySearchEnabled) {
		for (; i < iterationCount; i++) {

			step *= 0.5;
			marchingPosition = marchingPosition - step * sign(delta);

			screenPosition = generateProjectedPosition(marchingPosition);
			depthFromScreen = abs(generatePositionFromDepth(screenPosition, textureLod(textureDepth, screenPosition, 2).x).z);
			delta = abs(marchingPosition.z) - depthFromScreen;

			vec2 dCoords = smoothstep(0.2, 0.6, abs(vec2(0.5, 0.5) - screenPosition.xy));
			float screenEdgefactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);
			float ReflectionMultiplier = pow(Metallic, 3.0) * screenEdgefactor *
				-ReflectedVector.z;

			if (abs(delta) < distanceBias) {
				return texture(textureFrame, screenPosition).xyz * clamp(ReflectionMultiplier, 0.0, 0.9);
			}
		}
	}

	return vec3(0.0);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main() {
	vec3 position = generatePositionFromDepth(UV, textureLod(textureDepth, UV, 2).x);
	vec4 normal = view * vec4(texture(textureNorm, UV).xyz * 2.0 - 1.0, 0.0);

	float metallic = texture(textureMetallic, UV).r;
	Metallic = metallic;
	vec3 albedo = texture(textureAlbedo, UV).rgb;
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);
	vec3 Fresnel = fresnelSchlick(max(dot(normalize(normal.rgb), normalize(position)), 0.0), F0);

	if (!enableSSR || metallic < 0.01) {
		discard;// outColor = mix(texture(textureFrame, UV), normal, 0.9);
	}
	else {
		vec3 reflectionDirection = normalize(reflect(position, normalize(normal.xyz)));

		ReflectedVector = reflectionDirection;
		if (isSamplingEnabled) {
			vec3 firstBasis = normalize(cross(vec3(0.f, 0.f, 1.f), reflectionDirection));
			vec3 secondBasis = normalize(cross(reflectionDirection, firstBasis));
			vec4 resultingColor = vec4(0.f);
			for (int i = 0; i < sampleCount; i++) {
				vec2 coeffs = vec2(random(UV + vec2(0, i)) + random(UV + vec2(i, 0))) * samplingCoefficient;
				vec3 reflectionDirectionRandomized = reflectionDirection + firstBasis * coeffs.x + secondBasis * coeffs.y;
				vec3 tempColor = SSR(position, normalize(reflectionDirectionRandomized));

				if(tempColor != vec3(0))
					resultingColor += vec4(tempColor, 1.0f);
			}

			if(resultingColor.w != 0)
				resultingColor /= resultingColor.w;

			if (resultingColor.xyz == vec3(0.0f))
			{
				outColor = vec4(1, 0, 0, 0);

				return;
			}
			

			outColor = vec4(resultingColor.xyz * Fresnel, 1.0) ;
			
		}
		else {
			outColor = vec4(SSR(position, normalize(reflectionDirection)) * Fresnel, 1.0);
			if (outColor.xyz == vec3(0.f)) {
				outColor = texture(textureFrame, UV);
			}
		}
	}
}

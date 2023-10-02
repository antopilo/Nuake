#shader vertex
#version 440 core

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 UVPosition;

out vec2 UV;

void main()
{
    UV = UVPosition;
    gl_Position = vec4(VertexPosition, 1.0f);
}

#shader fragment
#version 440 core

layout(location = 0) out vec4 FragColor;

in vec2 UV;

uniform float SurfaceRadius;
uniform float AtmosphereRadius;
uniform vec3 RayleighScattering;
uniform vec3 MieScattering;
vec3 bMe = MieScattering * 1.1;
uniform float SunIntensity;
uniform vec3 CenterPoint;
uniform vec3 SunDirection;
uniform vec3 CamDirection;
uniform vec3 CamUp;
uniform vec3 CamRight;
uniform float u_Exposure;

// Calculate densities $\rho$.
// Returns vec2(rho_rayleigh, rho_mie)
// Note that intro version is more complicated and adds clouds by abusing Mie scattering density. That's why it's a separate function
vec2 densitiesRM(vec3 p) {
	float h = max(0., length(p - CenterPoint) - SurfaceRadius); // calculate height from Earth surface
	return vec2(exp(-h / 8e3), exp(-h / 12e2));
}

// Basically a ray-sphere intersection. Find distance to where rays escapes a sphere with given radius.
// Used to calculate length at which ray escapes atmosphere
float escape(vec3 p, vec3 d, float R) {
	vec3 v = p - CenterPoint;
	float b = dot(v, d);
	float det = b * b - dot(v, v) + R * R;
	if (det < 0.) return -1.;
	det = sqrt(det);
	float t1 = -b - det, t2 = -b + det;
	return (t1 >= 0.) ? t1 : t2;
}

// Calculate density integral for optical depth for ray starting at point `p` in direction `d` for length `L`
// Perform `steps` steps of integration
// Returns vec2(depth_int_rayleigh, depth_int_mie)
vec2 scatterDepthInt(vec3 o, vec3 d, float L, float steps) {
	// Accumulator
	vec2 depthRMs = vec2(0.);

	// Set L to be step distance and pre-multiply d with it
	L /= steps; d *= L;

	// Go from point P to A
	for (float i = 0.; i < steps; ++i)
		// Simply accumulate densities
		depthRMs += densitiesRM(o + d * i);

	return depthRMs * L;
}


// Global variables, needed for size
vec2 totalDepthRM;
vec3 I_R, I_M;

// Calculate in-scattering for ray starting at point `o` in direction `d` for length `L`
// Perform `steps` steps of integration
void scatterIn(vec3 o, vec3 d, float L, float steps) {

	// Set L to be step distance and pre-multiply d with it
	L /= steps; d *= L;

	// Go from point O to B
	for (float i = 0.; i < steps; ++i) {

		// Calculate position of point P_i
		vec3 p = o + d * i;

		// Calculate densities
		vec2 dRM = densitiesRM(p) * L;

		// Accumulate T(P_i -> O) with the new P_i
		totalDepthRM += dRM;

		// Calculate sum of optical depths. totalDepthRM is T(P_i -> O)
		// scatterDepthInt calculates integral part for T(A -> P_i)
		// So depthRMSum becomes sum of both optical depths
		vec2 depthRMsum = totalDepthRM + scatterDepthInt(p, SunDirection, escape(p, SunDirection, AtmosphereRadius), 4.);

		// Calculate e^(T(A -> P_i) + T(P_i -> O)
		vec3 A = exp(-RayleighScattering * depthRMsum.x - bMe * depthRMsum.y);

		// Accumulate I_R and I_M
		I_R += A * dRM.x;
		I_M += A * dRM.y;
	}
}

// Final scattering function
// O = o -- starting point
// B = o + d * L -- end point
// Lo -- end point color to calculate extinction for
vec3 scatter(vec3 o, vec3 d, float L, vec3 Lo) {

	// Zero T(P -> O) accumulator
	totalDepthRM = vec2(0.);
	
	// Zero I_M and I_R
	I_R = I_M = vec3(0.);
	
	// Compute T(P -> O) and I_M and I_R
	scatterIn(o, d, L, 16.);
	
	// mu = cos(alpha)
	float mu = dot(d, SunDirection);
	
	// Calculate Lo extinction
	return Lo * exp(-RayleighScattering * totalDepthRM.x - bMe * totalDepthRM.y)
	
		// Add in-scattering
		+ SunIntensity * (1. + mu * mu) * (
			I_R * RayleighScattering * .0597 +
			I_M * MieScattering * .0196 / pow(1.58 - 1.52 * mu, 1.5));
	return vec3(1.0);
}

uniform mat4 Projection;
uniform mat4 View;
void main()
{
	// Might add camera position in here.
	// It was already in there, I removed it.
	vec3 O = vec3(0., 0., 0.);
	
	vec2 NDC = UV * 2.0 - 1;
	mat4 newView = View;
	newView[3] = vec4(0,0,0,1);
	vec4 camSpace = inverse(Projection * newView) * vec4(vec3(NDC, 1.0), 1.0);
	vec3 direction = normalize(camSpace.xyz);
	vec3 D = direction;

	vec3 col = vec3(0.0);
	float L = escape(O, D, AtmosphereRadius);
	col = scatter(O, D, L, col);

	FragColor = vec4(sqrt(col * 4.0), 1.);
}
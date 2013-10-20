#version 330

// uniforms
layout(shared) uniform Camera
{
	mat4 ViewMatrix;
	mat4 ViewProjection;
	mat4 InverseViewProjection;
	vec3 CameraPosition;
};

//uniform vec3 LightDirection;
const vec3 LightDirection = vec3(0, -0.333, 0.333);

// input
in vec2 vs_out_texcoord;

// output
out vec4 fragColor;

// ------------------------------------------------
// SKY
// ------------------------------------------------
const vec3 AdditonalSunColor = vec3(1.0, 0.98, 0.8)/3;
const vec3 LowerHorizonColour = vec3(0.815, 1.141, 1.54)/2;
const vec3 UpperHorizonColour = vec3(0.986, 1.689, 2.845)/2;
const vec3 UpperSkyColour = vec3(0.16, 0.27, 0.43)*0.8;
const vec3 GroundColour = vec3(0.31, 0.41, 0.5)*0.8;
const float LowerHorizonHeight = -0.4;
const float UpperHorizonHeight = -0.1;
const float SunAttenuation = 2;
vec3 computeSkyColor(in vec3 ray)
{
	vec3 color;

	// background
	float heightValue = ray.y;	// mirror..
	if(heightValue < LowerHorizonHeight)
		color = mix(GroundColour, LowerHorizonColour, (heightValue+1) / (LowerHorizonHeight+1));
	else if(heightValue < UpperHorizonHeight)
		color = mix(LowerHorizonColour, UpperHorizonColour, (heightValue-LowerHorizonHeight) / (UpperHorizonHeight - LowerHorizonHeight));
	else
		color = mix(UpperHorizonColour, UpperSkyColour, (heightValue-UpperHorizonHeight) / (1.0-UpperHorizonHeight));
	
	// Sun
	float angle = max(0, dot(ray, LightDirection));
	color += (pow(angle, SunAttenuation) + pow(angle, 10000)*10) * AdditonalSunColor;

	return color;
}

// ------------------------------------------------
// MAIN
// ------------------------------------------------
void main()
{
	// "picking" - compute raydirection
	vec2 deviceCor = 2.0 * vs_out_texcoord - 1.0;
	vec4 rayOrigin = vec4(deviceCor, -1, 1) * InverseViewProjection;
	rayOrigin.xyz /= rayOrigin.w;
	vec4 rayTarget = vec4(deviceCor, 0, 1) * InverseViewProjection;
	rayTarget.xyz /= rayTarget.w;
	vec3 rayDirection = normalize(rayTarget.xyz - rayOrigin.xyz);

	// Color
	fragColor.a = 0.0;
	fragColor.rgb = computeSkyColor(rayDirection);
}

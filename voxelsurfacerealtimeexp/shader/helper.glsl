vec3 ComputeRayDirection(in vec2 screenTexCor, in mat4 InverseViewProjection)
{
	vec2 deviceCor = 2.0f * screenTexCor - 1.0f;
	vec4 rayOrigin = vec4(deviceCor, -1.0f, 1.0f) * InverseViewProjection;
	rayOrigin.xyz /= rayOrigin.w;
	vec4 rayTarget = vec4(deviceCor, -0.95f, 1.0f) * InverseViewProjection;
	rayTarget.xyz /= rayTarget.w;
	return normalize(rayTarget.xyz - rayOrigin.xyz);
}
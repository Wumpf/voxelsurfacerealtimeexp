vec3 ComputeRayDirection(in vec2 screenTexCor, in mat4 inverseViewProjection)
{
	vec2 deviceCor = 2.0f * screenTexCor - 1.0f;
	vec4 rayOrigin = inverseViewProjection * vec4(deviceCor, -1.0f, 1.0f);
	rayOrigin.xyz /= rayOrigin.w;
	vec4 rayTarget = inverseViewProjection * vec4(deviceCor, 0.0f, 1.0f) ;
	rayTarget.xyz /= rayTarget.w;
	return normalize(rayTarget.xyz - rayOrigin.xyz);
}

// http://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
bool rayBoxIntersection(in vec3 origin, in vec3 dir, in vec3 boxMin, in vec3 boxMax, out float tmin, out float tmax)
{
	vec3 invDir = 1.0f / dir;

	vec3 mins = (boxMin - origin)*invDir;
	vec3 maxs = (boxMax - origin)*invDir;

	tmin = max(max(min(mins.x, maxs.x), min(mins.y, maxs.y)), min(mins.z, maxs.z));
	tmax = min(min(max(mins.x, maxs.x), max(mins.y, maxs.y)), max(mins.z, maxs.z));

	return tmin <= tmax && tmax > 0;
}

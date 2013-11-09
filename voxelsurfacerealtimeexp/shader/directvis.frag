#version 430

#include "constantbuffers.glsl"
#include "helper.glsl"

layout(binding = 0) uniform sampler3D VolumeTexture;

// input
in vec2 vs_out_texcoord;

layout(binding = 0) readonly buffer SparseVoxelOctree_type
{
	uint Data[];
} SparseVoxelOctree;

// output
layout(location = 0, index = 0) out vec4 ps_out_fragColor;


const uint NumInitialNodesPerAxis = 8;
const uint InitialNodeSize = 64;
const uint InitialDepth = 3;

float SampleOctree(uvec3 volumePosition, out uint outLastBoxSize)
{
	// Read inital node
	outLastBoxSize = InitialNodeSize;
	uvec3 initalNodePos = volumePosition / outLastBoxSize;
	uint initialNodeIndex = (initalNodePos.x + (initalNodePos.y + initalNodePos.z * NumInitialNodesPerAxis) * NumInitialNodesPerAxis);
	uint currentNodeAdress = initialNodeIndex * 8;


	while(true)
	{
		uvec3 parentBoxMin = (volumePosition / outLastBoxSize) * outLastBoxSize;

		// Next box is half the size!
		outLastBoxSize /= uint(2);

		// In which subbox is volumePosition? True means positive, false negative.
		uvec3 subBoxCord = uvec3(greaterThanEqual(volumePosition - parentBoxMin, uvec3(outLastBoxSize)));
		// Compute where to which node to jump
		currentNodeAdress += subBoxCord.x + subBoxCord.y * 2 + subBoxCord.z * 4;

		// Read current node data and decide what to do next.
		uint currentData = SparseVoxelOctree.Data[currentNodeAdress];
		// The end?
		if(outLastBoxSize == 1)
			return uintBitsToFloat(currentData);
		else
		{
			// Nothing in it?
			if(currentData == 0)
				return -1.0f;
			// Filled?
			if(currentData == uint(0xFFFFFFFF))
				return 1.0f;
			// It's a new adress! Jump!
			else
				currentNodeAdress = currentData;
		}
	}
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

void main()
{
	vec3 rayDirection = ComputeRayDirection(vs_out_texcoord, InverseViewProjection);

	// extremly primitive raymarching
	float t0, t1;
	if(!rayBoxIntersection(CameraPosition, rayDirection, vec3(0), vec3(VolumeWorldSize), t0, t1))
		discard;

	float start = max(5, min(t0, t1));
	float end = max(t0, t1);

    uvec3 lastSamplePosUInt = uvec3(9999999);
    float nextStep;
	for(float t=start; t<end; t+=nextStep)
	{
		nextStep = t * 0.01f; // default step - this is min!

		vec3 samplePos = CameraPosition + rayDirection * t;

		uvec3 samplePosUInt = uvec3(samplePos + vec3(0.5,0.5,0.5));
		if(all(samplePosUInt == lastSamplePosUInt))
			continue;
		lastSamplePosUInt = samplePosUInt;

		uint lastBoxSize;
		float density = SampleOctree(samplePosUInt, lastBoxSize);

	//	vec4 vol = textureLod(VolumeTexture, samplePos / VolumeWorldSize, 0);
		if(density > 0.0f)
		{
/*			vec3 normal = normalize(vol.xyz);
			normal = normalize(normal);
			float lighting = clamp(dot(normal, GlobalDirLightDirection), 0, 1);
			ps_out_fragColor.xyz = GlobalDirLightColor * lighting + GlobalAmbient;
			ps_out_fragColor.a = 1.0f;

	//		ps_out_fragColor.xyz = abs(rayDirection);//abs(normal);

*/
			// initial cell coloring
			uint currentBoxSize = InitialNodeSize;
			uvec3 initalNodePos = samplePosUInt / InitialNodeSize;
			uint initialNodeIndex = (initalNodePos.x + (initalNodePos.y + initalNodePos.z * NumInitialNodesPerAxis) * NumInitialNodesPerAxis);
			ps_out_fragColor.xyz = vec3(initalNodePos) / NumInitialNodesPerAxis;

			return;
		}
		/*else if(lastBoxSize > nextStep * 4)
		{
			vec3 boxMin = (uvec3(samplePos) / uvec3(lastBoxSize)) + vec3(lastBoxSize);
			vec3 boxMax = boxMin + vec3(lastBoxSize);
			rayBoxIntersection(samplePos, rayDirection, boxMin, boxMax, t0, t1);

			nextStep = max(min(t0, t1), nextStep);
		}*/
	}

	discard;
}

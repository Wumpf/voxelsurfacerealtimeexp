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
const uint InitialNodeSize = 16;
const uint InitialDepth = 3;

float SampleOctree(uvec3 volumePosition, out uint outCurrentBoxSize)
{
	// Read inital node
	outCurrentBoxSize = InitialNodeSize;
	uvec3 initalNodePos = volumePosition / outCurrentBoxSize;
	uint initialNodeIndex = (initalNodePos.x + (initalNodePos.y + initalNodePos.z * NumInitialNodesPerAxis) * NumInitialNodesPerAxis);
	uint currentNodeAdress = initialNodeIndex * 8;

	while(true)
	{
		vec3 inBoxCoord = mod(volumePosition, vec3(outCurrentBoxSize));

		// Next box is half the size!
		outCurrentBoxSize /= uint(2);

		// In which subbox is volumePosition? True means positive, false negative.
		uvec3 subBoxCord = uvec3(greaterThanEqual(inBoxCoord, uvec3(outCurrentBoxSize)));
		// Compute to which node to jump
		currentNodeAdress += subBoxCord.x + subBoxCord.y * 2 + subBoxCord.z * 4;

		// Read current node data and decide what to do next.
		uint currentData = SparseVoxelOctree.Data[currentNodeAdress];
		// The end?
		if(outCurrentBoxSize == 1)
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

    float nextStep;
	for(float t=start; t<end; t+=nextStep)
	{
		nextStep = t * 0.01f; // default step - this is min!

		vec3 samplePos = CameraPosition + rayDirection * t;
		vec3 interp = fract(samplePos);


		uvec3 samplePosUInt = uvec3(samplePos);

		uint lastBoxSize;
		float density000 = SampleOctree(samplePosUInt + uvec3(0, 0, 0), lastBoxSize);
		float density100 = SampleOctree(samplePosUInt + uvec3(1, 0, 0), lastBoxSize);
		float density010 = SampleOctree(samplePosUInt + uvec3(0, 1, 0), lastBoxSize);
		float density110 = SampleOctree(samplePosUInt + uvec3(1, 1, 0), lastBoxSize);
		float density001 = SampleOctree(samplePosUInt + uvec3(0, 0, 1), lastBoxSize);
		float density101 = SampleOctree(samplePosUInt + uvec3(1, 0, 1), lastBoxSize);
		float density011 = SampleOctree(samplePosUInt + uvec3(0, 1, 1), lastBoxSize);
		float density111 = SampleOctree(samplePosUInt + uvec3(1, 1, 1), lastBoxSize);

		float density = mix(mix(mix(density000, density100, interp.x), mix(density010, density110, interp.x), interp.y),
						    mix(mix(density001, density101, interp.x), mix(density011, density111, interp.x), interp.y), interp.z);

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
